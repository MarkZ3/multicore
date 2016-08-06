#include <QDebug>
#include <pthread.h>
#include <cassert>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

struct arg {
    int delay;
    int repeat;
};

int a;
void *worker_mutex(void *args)
{
    struct arg *arg = (struct arg *) args;

    /* Synchronisation avec un verrou */
    pthread_mutex_lock(&lock);
    for (int i = 0; i < arg->repeat; i++) {
        a++;
    }
    pthread_mutex_unlock(&lock);
    pthread_barrier_wait(&barrier);
    return NULL;
}

int b;
void *worker_delay(void *args)
{
    struct arg *arg = (struct arg *) args;

    /* Synchronisation avec un délai */
    qDebug() << arg->delay;
    struct timespec ts = { arg->delay, 0 };
    nanosleep(&ts, NULL);

    for (int i = 0; i < arg->repeat; i++) {
        b++;
    }
    pthread_barrier_wait(&barrier);
    return NULL;
}

int c;
void *worker_race(void *args)
{
    struct arg *arg = (struct arg *) args;

    for (int i = 0; i < arg->repeat; i++) {
        c++;
    }
    pthread_barrier_wait(&barrier);
    return NULL;
}

struct experiment {
    const char *name;
    void* (*worker)(void *);
};

static struct experiment exp[] = {
    { .name = "race", .worker = worker_race },
    { .name = "delay", .worker = worker_delay },
    { .name = "mutex", .worker = worker_mutex },
    { .name = NULL, .worker = NULL },
};

void execute_exp(struct experiment *exp)
{
    int n = 2;
    int repeat = 100;
    pthread_t t[n];
    struct arg args[n];

    pthread_barrier_init(&barrier, NULL, n + 1);

    qDebug() << "START" << exp->name;

    for (int i = 0; i < n; i++) {
        args[i].delay = i;
        args[i].repeat = repeat;
        pthread_create(&t[i], NULL, exp->worker, &args[i]);
    }

    pthread_barrier_wait(&barrier);

    for (int i = 0; i < n; i++) {
        pthread_join(t[i], NULL);
    }

    qDebug() << "DONE " << exp->name << "\n";
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    for (int i = 0; exp[i].name != NULL; i++) {
        execute_exp(&exp[i]);
    }

    qDebug() << "a" << a;
    qDebug() << "b" << b;
    qDebug() << "c" << c;

    return 0;
}
