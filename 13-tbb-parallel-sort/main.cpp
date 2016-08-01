#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>

#include <QVector>
#include <QDebug>
#include <QElapsedTimer>

#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>

double elapsed(std::function<void ()> func)
{
    QElapsedTimer timer;
    timer.start();
    func();
    return timer.nsecsElapsed() / 1000000000.0f;
}

void  print_result(QString name, double reference, double actual)
{
    qDebug() << QString("%1 %2s (%3x)")
                .arg(name, -12, QLatin1Char(' '))
                .arg(actual, 0, 'f', 6)
                .arg(reference / actual, 0, 'f', 2);
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    const int n = 1E6;

    QVector<int> ints(n);
    for (int i = 0; i < n; ++i) {
        ints[i] = i;
    }

    QVector<double> serial;
    QVector<double> parallel;
    for (int i = 0; i < 10; i++) {
        std::random_shuffle(ints.begin(), ints.end());
        double s = elapsed([&]() {
            std::sort(ints.begin(), ints.end());
        });
        std::random_shuffle(ints.begin(), ints.end());

        double p = elapsed([&]() {
            tbb::parallel_sort(ints.begin(), ints.end());
        });
        serial.push_back(s);
        parallel.push_back(p);
    }

    double sum_serial = std::accumulate(serial.begin(), serial.end(), 0.0, std::plus<double>());
    double sum_parallel = std::accumulate(parallel.begin(), parallel.end(), 0.0, std::plus<double>());

    print_result("serial", sum_serial, sum_serial);
    print_result("parallel", sum_serial, sum_parallel);

    return 0;
}
