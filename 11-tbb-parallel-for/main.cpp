#include <QDebug>
#include <QElapsedTimer>
#include <QVector>
#include "tbb/tbb.h"

/*
 * parallel implementation of memset()
 */

class MemSet {
public:
    MemSet(int *data): m_data(data) { }
    /*
     * The actual work is implemented into operator(), but the method has a body const.
     * It means we cannot change object members. They can be passed as raw pointers,
     * or marked mutable.
     */
    void operator() (const tbb::blocked_range<int>& range) const {
        for (int i = range.begin(); i < range.end(); i++) {
            m_data[i] = i;
        }
    }
private:
    int *m_data;
};

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
    int n = 1E6;
    QVector<int> array(n); // elements are initialized

    // serial
    double serial = elapsed([&]() {
        for (int i = 0; i < n; i++) {
            array[i] = i;
        }
    });

    // lambda
    double parallel = elapsed([&]() {
        tbb::parallel_for(tbb::blocked_range<int>(0, n),
            [&] (tbb::blocked_range<int>& range) {
                for (int i = range.begin(); i < range.end(); i++) {
                    array[i] = i;
                }
        });
    });

    // lambda without inner loop
    tbb::parallel_for(0, n, 1, [&] (int& i) { array[i] = i; } );

    // classic
    tbb::parallel_for(tbb::blocked_range<int>(0, n), MemSet(array.data()));

    print_result("serial", serial, serial);
    print_result("parallel", serial, parallel);

    return 0;
}
