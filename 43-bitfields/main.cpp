#include <QDebug>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t foo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bar = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

struct bitfield {
    uint x: 1;
    uint y: 1;
};

static struct bitfield value;

void *worker1(void *args)
{
    (void) args;


    pthread_mutex_lock(&foo);
    value.x = !value.x;
    pthread_mutex_unlock(&foo);

    pthread_barrier_wait(&barrier);
    return NULL;
}

void *worker2(void *args)
{
    (void) args;
    sleep(1);
    pthread_mutex_lock(&bar);
    value.y = !value.y;
    pthread_mutex_unlock(&bar);

    pthread_barrier_wait(&barrier);
    return NULL;
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    value.x = 0;
    value.y = 1;
    qDebug() << value.x << value.y;

    pthread_barrier_init(&barrier, NULL, 2);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker1, NULL);
    pthread_create(&t2, NULL, worker2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    qDebug() << value.x << value.y;

    return 0;
}
