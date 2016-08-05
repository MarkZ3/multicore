#include <QDebug>
#include <QVector>
#include <QElapsedTimer>

#include <mutex>
#include <random>
#include <immintrin.h>
#include <thread>
#include <chrono>
#include <urcu/uatomic.h>
#include <tbb/tbb.h>

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
            switch(m_code) {
            case _XABORT_EXPLICIT:
                uatomic_inc(&m_abort_reasons[0]);
                break;
            case _XABORT_RETRY:
                uatomic_inc(&m_abort_reasons[1]);
                break;
            case _XABORT_CONFLICT:
                uatomic_inc(&m_abort_reasons[2]);
                break;
            case _XABORT_CAPACITY:
                uatomic_inc(&m_abort_reasons[3]);
                break;
            case _XABORT_DEBUG:
                uatomic_inc(&m_abort_reasons[4]);
                break;
            case _XABORT_NESTED:
                uatomic_inc(&m_abort_reasons[5]);
                break;
            default:
                break;
            }
        } else {
            _xend();
            uatomic_inc(&m_success);
        }
    }

    uint get_code() {
        return m_code;
    }

    static void reset() {
        m_abort = 0;
        m_success = 0;
        for (int i = 0; i < 6; i++) {
            m_abort_reasons[i] = 0;
        }
    }

    static double abort_rate() {
        return m_abort / (double) (m_success + m_abort);
    }

    SpinLock *m_lock;
    uint m_code;
    static uint m_abort;
    static uint m_success;
    static uint m_abort_reasons[];
};

uint TransactionScope::m_abort = 0;
uint TransactionScope::m_success = 0;
uint TransactionScope::m_abort_reasons[6];

double elapsed(std::function<void ()> func)
{
    QElapsedTimer timer;
    timer.start();
    func();
    return timer.nsecsElapsed() / 1000000000.0f;
}

void print_result(QString name, double reference, double actual)
{
    qDebug() << QString("%1 %2s (%3x)")
                .arg(name, -12, QLatin1Char(' '))
                .arg(actual, 0, 'f', 6)
                .arg(reference / actual, 0, 'f', 2);
}

void do_transaction(QVector<int> &accounts, QVector<int> &rnd, int i)
{
    int amount = 100;
    int from = rnd[i % rnd.size()];
    int to = rnd[(i + 1) % rnd.size()];
    accounts[from] -= amount;
    accounts[to] += amount;
}

void do_transaction2(int *accounts, int from, int to)
{
    int amount = 100;
    accounts[from] -= amount;
    accounts[to] += amount;
}

typedef tbb::concurrent_vector<int> ConcurrentVector;

int main()
{
    if (!cpu_has_rtm()) {
        qDebug() << "rtm not supported";
        return -1;
    }

    std::random_device rd;
    std::mt19937 g(rd());

    int iter = 1E7;
    for (uint size = 1E2; size < 1E5; size *= 10) {
        QVector<int> accounts(size);
        QVector<int> rnd(size);
        for (uint x = 0; x < size; x++) {
            rnd[x] = x;
        }
        std::shuffle(rnd.begin(), rnd.end(), g);

        double serial = elapsed([&]() {
            for (int i = 0; i < iter; i++) {
                do_transaction(accounts, rnd, i);
            }
        });

        SpinLock lock;
        TransactionScope::reset();
        double parallel_rtm = elapsed([&]() {
            tbb::parallel_for(0, iter, [&](int &i) {
                int amount = 100;
                int from = rnd[i % rnd.size()];
                int to = rnd[(i + 1) % rnd.size()];
                int *data = accounts.data();
                TransactionScope scope(&lock);
                data[from] -= amount;
                data[to] += amount;
            });
        });

        qDebug() << "size=" << size;
        print_result("serial", serial, serial);
        print_result("parallel rtm", serial, parallel_rtm);
        qDebug() << "abort rate" << QString("%1").arg(TransactionScope::abort_rate(), 0, 'f', 2);
        for (int i = 0; i < 6; i++) {
            qDebug() << "abort reason " << i << TransactionScope::m_abort_reasons[i];
        }
    }

    return 0;
}

