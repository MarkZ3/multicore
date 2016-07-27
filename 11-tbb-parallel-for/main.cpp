#include <QDebug>
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

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    int n = 100;
    int *array = new int[n];

    // classic
    tbb::parallel_for(tbb::blocked_range<int>(0, n), MemSet(array));

    // lambda
    tbb::parallel_for(tbb::blocked_range<int>(0, n),
        [=] (tbb::blocked_range<int>& range) {
            for (int i = range.begin(); i < range.end(); i++) {
                array[i] = i;
            }
    });

    // lambda without inner loop
    tbb::parallel_for(0, n, 1, [=] (int& i) { array[i] = i; } );

    delete[] array;
}
