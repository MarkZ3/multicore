#include <QDebug>
#include <QVector>
#include <omp.h>

void process_sample(int i)
{
    static const QString msg[] = { "even", "odd" };
    qDebug() << omp_get_thread_num() << msg[i % 2] << i;
}


int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    int n = 100;
    QVector<int> v(n);
    for (int i = 0; i < n; i++) {
        v[i] = i;
    }

    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < n; i++) {

                // launch task in background
                #pragma omp task
                {
                    process_sample(i);
                }

                // just for fun, wait for a batch of task to complete
                if (((i + 1) % 10) == 0) {
                    #pragma omp taskwait
                    qDebug() << "taskwait done";
                }
            }
        }
    }

    qDebug() << "done";
    return 0;
}
