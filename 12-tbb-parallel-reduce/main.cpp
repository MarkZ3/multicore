#include <QDebug>
#include <QElapsedTimer>
#include <QMutex>
#include <thread>
#include "tbb/tbb.h"

using namespace std;

struct work {
    int n;
    int rank;
    QVector<int> *data;
    int local_sum;
};

/*
 * Source: https://software.intel.com/fr-fr/node/506154
 */

class Sum {
public:
    // default constructor
    Sum(int *data) : m_value(0), m_data(data) { }

    // split constructor: called to make copies of the object for each thread
    // in this case, the sum of a range starts at zero
    Sum(Sum& sum, tbb::split) : m_data(sum.m_data) {
        m_value = 0;
    }

    // implements operator() to reduce the provided range
    // the state of the object is updated
    void operator() (const tbb::blocked_range<int>& range) {
        for (int i = range.begin(); i < range.end(); i++) {
            m_value += m_data[i];
        }
        return ;
    }

    // join() collapsed the result of two consecutive ranges
    void join(Sum& right) {
        m_value += right.m_value;
    }
    int m_value;
    int *m_data;
};

int fast_sum(QVector<int> &data)
{
    Sum sum(data.data());
    tbb::parallel_reduce(tbb::blocked_range<int>(0, data.length()), sum);
    return sum.m_value;
}

void do_sum(work *arg)
{
    int begin = arg->data->size() * arg->rank / arg->n;
    int end = arg->data->size() * (arg->rank + 1) / arg->n;
    int *v = arg->data->data();
    int sum = 0;
    for (int i = begin; i < end; i++) {
        sum += v[i];
    }
    arg->local_sum = sum;
}

int reduce_pthread(QVector<int> &v, int n)
{
    thread threads[n];
    work items[n];
    int final_sum = 0;

    for (int i = 0; i < n; i++) {
        items[i] = work{n, i, &v, 0};
        threads[i] = thread(do_sum, &items[i]);
    }

    for (int i = 0; i < n; i++) {
        threads[i].join();
        final_sum += items[i].local_sum;
    }
    return final_sum;
}

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

int main()
{
    int n = 1E7;
    QVector<int> v(n);

    // initialization
    for (int i = 0; i < n; i++) {
        v[i] = i;
    }

    // serial
    int exp = 0;
    double elapsed_serial = elapsed([&]() {
        for (int i = 0; i < n; i++) {
            exp += v[i];
        }
    });

    // reduction with parallel_for
    int sum0 = 0;
    double elapsed_race = elapsed([&]() {
        tbb::parallel_for(0, v.size(), [&](int &i) {
            sum0 += v[i]; // race condition!
        });
    });

    qDebug() << "exp" << exp << "act" << sum0;

    // reduction with parallel_for
    sum0 = 0;
    QMutex mutex;
    double elapsed_lock = elapsed([&]() {
        tbb::parallel_for(0, v.size(), [&](int &i) {
            mutex.lock();
            sum0 += v[i];
            mutex.unlock();
        });
    });

    qDebug() << "exp" << exp << "act" << sum0;

    // reduction without lock
    int sum1 = 0;
    int cpus = thread::hardware_concurrency();
    double elapsed_pthread = elapsed([&]() {
        sum1 = reduce_pthread(v, cpus);
    });
    qDebug() << "exp" << exp << "act" << sum1;

    // lambda
    int sum2 = tbb::parallel_reduce(
        tbb::blocked_range<int>(0, n),  // global range to process
        0,                              // initial value

        // first lambda: compute the reduce for a subrange
        [&] (const tbb::blocked_range<int>& range, int sum) -> int {
            for (int i = range.begin(); i < range.end(); i++) {
                sum += v[i];
            }
            return sum;
        },

        // second lambda: merge range results
        [] (int x, int y) -> int {
            return x + y;
        }
    );

    qDebug() << "exp" << exp << "act" << sum2;

    // classic parallel_reduce
    int sum3 = 0;
    double elapsed_tbb = elapsed([&]() {
        sum3 = fast_sum(v);
    });
    qDebug() << "exp" << exp << "act" << sum3;

    print_result("serial", elapsed_serial, elapsed_serial);
    print_result("race", elapsed_serial, elapsed_race);
    print_result("lock", elapsed_serial, elapsed_lock);
    print_result("pthread", elapsed_serial, elapsed_pthread);
    print_result("tbb", elapsed_serial, elapsed_tbb);

    return 0;
}
