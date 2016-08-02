#include <QDebug>
#include <QVector>

#include <mutex>
#include <random>
#include <immintrin.h>
#include <thread>
#include <chrono>
#include <urcu/uatomic.h>

#include "tsx-cpuid.h"

class SpinLock
{
    unsigned int state;
    enum { Unlocked = 0, Locked = 1};
public:
    SpinLock() : state(Unlocked) { }

    void lock() {
        while(uatomic_cmpxchg(&state, Unlocked, Locked) != Unlocked) {
            do {
                _mm_pause();
            } while(state == Locked);
        }
    }

    void unlock() {
        uatomic_xchg(&state, Unlocked);
    }

    bool isLocked() const {
        return CMM_ACCESS_ONCE(state) == Locked;
    }

};

class TransactionScope {
public:
    TransactionScope(SpinLock *lock) : m_lock(lock), m_code(0) {
        m_code = _xbegin();
        if (m_code == _XBEGIN_STARTED) {
            if (!m_lock->isLocked()) {
                return;
            } else {
                _xabort(1);
            }
        } else {
            m_lock->lock();
        }
    }

    ~TransactionScope() {
        if (m_lock->isLocked()) {
            m_lock->unlock();
        } else {
            _xend();
        }
    }

    uint get_code() {
        return m_code;
    }

    SpinLock *m_lock;
    uint m_code;
};

volatile int run;

void worker(SpinLock *lock, QVector<int> *data, int amount)
{
    int *buf = data->data();
    uint size = data->size();
    uint i = 0;
    while(run) {
        TransactionScope scope(lock);
        buf[i++ % size] += amount;
    }
}

int main()
{
    SpinLock lock;
    int n = 1E3;
    QVector<int> data(n);

    if (!cpu_has_rtm()) {
        qDebug() << "rtm not supported";
        return -1;
    }

    run = 1;
    __sync_synchronize();

    std::thread t1(worker, &lock, &data, -1);
    std::thread t2(worker, &lock, &data, 1);

    std::chrono::milliseconds delay(1000);
    std::this_thread::sleep_for(delay);

    run = 0;
    __sync_synchronize();

    t1.join();
    t2.join();
    return 0;
}

