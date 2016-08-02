#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <omp.h>

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    /*
     * Example 1: omp parallel
     */
    qDebug() << "omp parallel begin";
    #pragma omp parallel
    {
        qDebug() << "rank" << omp_get_thread_num() <<
                    "size" << omp_get_num_threads();
    }
    qDebug() << "omp parallel end";

    /*
     * Example 2: omp parallel sections
     */
    qDebug() << "omp parallel sections begin";
    #pragma omp parallel
    {
        #pragma omp sections
        {
            qDebug() << "section 1 rank" << omp_get_thread_num();
        }
        #pragma omp sections
        {
            qDebug() << "section 2 rank" << omp_get_thread_num();
        }
        #pragma omp sections
        {
            qDebug() << "section 3 rank" << omp_get_thread_num();
        }
    }
    qDebug() << "omp parallel sections end";

    /*
     * Example 3: omp task
     */
    qDebug() << "omp task begin";
    QElapsedTimer timer;
    timer.start();
    #pragma omp task
    {
        qDebug() << "task 1 rank" << omp_get_thread_num();
        QThread::usleep(1000);
    }
    #pragma omp task
    {
        qDebug() << "task 2 rank" << omp_get_thread_num();
        QThread::usleep(1000);
    }
    #pragma omp barrier
    qDebug() << "omp task end" << timer.elapsed();

    /*
     * Example 4: omp parallel single
     */
    qDebug() << "omp parallel single begin";
    #pragma omp parallel
    {
        #pragma omp single
        {
            qDebug() << "rank" << omp_get_thread_num();
        }
    }
    qDebug() << "omp parallel single end";

    /*
     * Example 5: omp parallel master
     */
    qDebug() << "omp parallel master begin";
    #pragma omp parallel
    {
        #pragma omp master
        {
            qDebug() << "rank" << omp_get_thread_num();
        }
    }
    qDebug() << "omp parallel master end";

}
