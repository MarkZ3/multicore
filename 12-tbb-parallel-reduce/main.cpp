#include <QDebug>
#include "tbb/tbb.h"

/*
 * Source: https://software.intel.com/fr-fr/node/506154
 */

class Sum {
public:
    // default constructor
    Sum(QVector<int>& data) : m_value(0), m_data(data) { }

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
    QVector<int>& m_data;
};

int fast_sum(QVector<int> data)
{
    Sum sum(data);
    tbb::parallel_reduce(tbb::blocked_range<int>(0, data.length()), sum);
    return sum.m_value;
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    int n = 100;
    QVector<int> v(n);

    int exp = 0;
    for (int i = 0; i <n; i++) {
        v[i] = i;
        exp += i;
    }

    // classic parallel_reduce


    // lambda
    int sum = tbb::parallel_reduce(
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

    qDebug() << "exp" << exp << "act" << sum;

    sum = fast_sum(v);
    qDebug() << "exp" << exp << "act" << sum;

    return 0;
}
