#include <pthread.h>
#include <unistd.h>

pthread_mutex_t foo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bar = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

int x = 0;

void *worker1(void *args)
{
    (void) args;

    pthread_mutex_lock(&foo);
    pthread_mutex_lock(&bar);
    x++;
    pthread_mutex_unlock(&foo);
    pthread_mutex_unlock(&bar);
    pthread_barrier_wait(&barrier);
    return NULL;
}

void *worker2(void *args)
{
    (void) args;
    sleep(1);
    pthread_mutex_lock(&bar);
    pthread_mutex_lock(&foo);
    x++;
    pthread_mutex_unlock(&bar);
    pthread_mutex_unlock(&foo);

    pthread_barrier_wait(&barrier);
    return NULL;
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    pthread_barrier_init(&barrier, NULL, 2);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker1, NULL);
    pthread_create(&t2, NULL, worker2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
