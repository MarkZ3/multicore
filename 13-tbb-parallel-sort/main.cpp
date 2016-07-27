#include <QDebug>
#include <algorithm>
#include "tbb/tbb.h"

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    int n = 10;
    QVector<int> data(n);

    for (int i = 0; i < n; i++) {
        data[i] = i;
    }

    std::random_shuffle(data.begin(), data.end());
    qDebug() << "rand" << data;

    tbb::parallel_sort(data.begin(), data.end());
    qDebug() << "incr" << data;

    tbb::parallel_sort(data.begin(), data.end(), std::greater<int>());
    qDebug() << "desc" << data;

    return 0;
}
