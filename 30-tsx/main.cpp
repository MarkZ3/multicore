#include <QDebug>
#include <QVector>
#include <QElapsedTimer>

#include <mutex>
#include <random>
#include <immintrin.h>
#include <thread>
#include <chrono>
#include <urcu/uatomic.h>

#include "tsx-cpuid.h"

/*
 * Based on: https://software.intel.com/en-us/blogs/2012/11/06/exploring-intel-transactional-synchronization-extensions-with-intel-software
 */

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
            uatomic_inc(&m_abort);
        } else {
            _xend();
            uatomic_inc(&m_success);
        }
    }

    uint get_code() {
        return m_code;
    }

    SpinLock *m_lock;
    uint m_code;
    static uint m_abort;
    static uint m_success;
};

uint TransactionScope::m_abort = 0;
uint TransactionScope::m_success = 0;

volatile int run;

void worker(SpinLock *lock, QVector<int> *data, int amount)
{
    int *buf = data->data();
    uint size = data->size();
    uint i = 0;
    std::random_device rd;
    std::mt19937 g(rd());
    QVector<int> rnd(size);
    for (uint x = 0; x < size; x++) {
        rnd[x] = x;
    }
    std::shuffle(rnd.begin(), rnd.end(), g);

    while(run) {
        TransactionScope scope(lock);
        buf[rnd[i++ % size]] += amount;
    }
}

void do_rtm(int n, int msdelay)
{
    QVector<int> data(n);
    SpinLock lock;

    run = 1;
    TransactionScope::m_abort = 0;
    TransactionScope::m_success = 0;
    __sync_synchronize();

    std::thread t1(worker, &lock, &data, -1);
    std::thread t2(worker, &lock, &data, 1);

    std::chrono::milliseconds delay(msdelay);
    std::this_thread::sleep_for(delay);

    run = 0;
    __sync_synchronize();

    t1.join();
    t2.join();

    double abort_rate = 100 * TransactionScope::m_abort / (double) TransactionScope::m_success;
    long tx = TransactionScope::m_success + TransactionScope::m_abort;

    qDebug() << "transactions" << tx;
    qDebug() << "abort rate" << QString("%1").arg(abort_rate, 0, 'f', 2);
}

int main()
{
    if (!cpu_has_rtm()) {
        qDebug() << "rtm not supported";
        return -1;
    }

    int msdelay = 1000;
    for (int pwr = 1E2; pwr < 1E5; pwr *= 10) {
        do_rtm(pwr, msdelay);
    }

    return 0;
}

