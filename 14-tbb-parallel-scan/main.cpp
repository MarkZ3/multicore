#include <QDebug>
#include <QVector>
#include <QElapsedTimer>
#include <cassert>
#include <tbb/tbb.h>

void prefix_sum_serial(const QVector<int> &a, QVector<int> &b)
{
    int len = std::min(a.size(), b.size());
    if (len == 0)
        return;
    b[0] = a[0];
    for (int i = 1; i < len; i++) {
        b[i] = b[i - 1] + a[i];
    }
}

class PrefixSumParallel {
    int sum;
    QVector<int> *a;
    QVector<int> *b;
public:
    PrefixSumParallel(QVector<int> *a_, QVector<int> *b_) :
        sum(0), a(a_), b(b_) { }

    int get_sum() const {
        return sum;
    }

    template<typename Tag>
    void operator()(const tbb::blocked_range<int>& range, Tag) {
        int temp = sum;
        for (int i = range.begin(); i < range.end(); i++) {
            temp = temp + a->at(i);
            if( Tag::is_final_scan() )
                b->replace(i, temp);
        }
        sum = temp;
    }

    PrefixSumParallel(PrefixSumParallel& other, tbb::split) :
        sum(0), a(other.a), b(other.b) { }

    void reverse_join(PrefixSumParallel& other) {
        sum = other.sum + sum;
    }

    void assign(PrefixSumParallel& other) {
        sum = other.sum;
    }
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
    int len = 1E8;
    QVector<int> a(len);
    QVector<int> b(len);
    QVector<int> c(len);

    int count = 0;
    std::for_each(a.begin(), a.end(), [&](int &x) { x = count++; });
    std::fill(b.begin(), b.end(), 0);
    std::fill(c.begin(), c.end(), 0);

    double serial = elapsed([&]() {
        prefix_sum_serial(a, b);
    });

    PrefixSumParallel prefix_sum_parallel(&a, &c);
    double parallel = elapsed([&]() {
        tbb::parallel_scan(tbb::blocked_range<int>(0, len), prefix_sum_parallel);
    });

    tbb::parallel_for(0, len, [&](int i) {
        assert(b[i] == c[i]);
    });

    print_result("serial", serial, serial);
    print_result("tbb scan", serial, parallel);

    return 0;
}

