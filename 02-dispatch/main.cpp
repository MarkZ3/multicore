#include <QDebug>
#include <QElapsedTimer>
#include <QMap>
#include <QFile>
#include <thread>
#include <functional>
#include <iostream>
#include <tbb/tbb.h>

using namespace std;

// this algorithm has runtime of O(n^2)
int prefix_sum(int n)
{
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += i;
    }
    return total;
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
    int range_max = 50000;
    vector<int> result(range_max);
    auto prefix_range = [&] (const int x0, const int x1) {
        //qDebug() << "thread:" << x0 << x1;
        for (int x = x0; x < x1; x++) {
            result[x] = prefix_sum(x);
        }
    };

    double serial = elapsed([&](){ prefix_range(0, range_max); });

    double fixed = elapsed([&](){
        int n = thread::hardware_concurrency();
        vector<thread> t;
        for (int i = 0; i < n; i++) {
            int x0 = range_max * (i)     / n;
            int x1 = range_max * (i + 1) / n;
            t.push_back(thread(prefix_range, x0, x1));
        }

        for (int i = 0; i < n; i++) {
            t[i].join();
        }
    });

    double dynamic = elapsed([&](){
        tbb::parallel_for(tbb::blocked_range<int>(0, range_max),
            [&] (const tbb::blocked_range<int>& range) {
                prefix_range(range.begin(), range.end());
        });
    });

    // report
    print_result("serial", serial, serial);
    print_result("fixed", serial, fixed);
    print_result("dynamic", serial, dynamic);
    return 0;
}

