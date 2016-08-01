#include <QDebug>
#include <QVector>
#include <QMap>
#include <QElapsedTimer>

#include <atomic>
#include <cassert>
#include <functional>
#include <tbb/tbb.h>


struct HashCmp {
    static size_t hash(const int &x) {
        return std::hash<int>()(x);
    }
    static bool equal(const int &x, const int &y) {
        return x == y;
    }
};

typedef tbb::concurrent_hash_map<int, int, HashCmp> ConcurrentMap;

QDebug operator<<(QDebug debug, ConcurrentMap &map)
{
    QDebugStateSaver saver(debug);
    //debug.nospace() << '(' << c.x() << ", " << c.y() << ')';
    debug.nospace() << "ConcurrentMap(";
    for (ConcurrentMap::iterator i = map.begin(); i != map.end(); ++i) {
        debug.nospace() << '(' << i->first << ", " << i->second << ")";
    }
    debug.nospace() << ")";

    return debug;
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
    int modulo = 1000;
    QVector<int> data(n);

    // fill with random numbers
    // the number of keys is greater than the number of cpus
    for (int i = 0; i < data.size(); i++) {
        data[i] = rand() % modulo;
    }

    //qDebug() << data;

    // serial map
    QMap<int, int> serial_map;
    double elapsed_serial = elapsed([&]() {
        for (int i = 0; i < data.size(); i++) {
            serial_map[data[i]]++;
        }
    });

    ConcurrentMap parallel_map;
    double elapsed_parallel = elapsed([&]() {
        tbb::parallel_for(tbb::blocked_range<int>(0, n),
            [&](tbb::blocked_range<int> &range) {
                for (int i = range.begin(); i < range.end(); i++) {
                    ConcurrentMap::accessor accessor;
                    parallel_map.insert(accessor, data[i]);
                    accessor->second += 1;
                }
        });
    });

    std::atomic<int> atomic_vec[modulo];
    for (int i = 0; i < modulo; i++) {
        atomic_vec[i].store(0);
    }

    double elapsed_atomic = elapsed([&]() {
        tbb::parallel_for(tbb::blocked_range<int>(0, n),
            [&](tbb::blocked_range<int> &range) {
                for (int i = range.begin(); i < range.end(); i++) {
                    atomic_vec[data[i]]++;
                }
        });
    });

    for (const int key: serial_map.keys()) {
        ConcurrentMap::accessor accessor;
        parallel_map.find(accessor, key);
        assert(serial_map[key] ==  accessor->second);
        assert(serial_map[key] == atomic_vec[key]);
    }

    print_result("serial", elapsed_serial, elapsed_serial);
    print_result("parallel", elapsed_serial, elapsed_parallel);
    print_result("atomic", elapsed_serial, elapsed_atomic);

    return 0;
}

