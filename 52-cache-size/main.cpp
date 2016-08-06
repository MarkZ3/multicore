#include <QDebug>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/syscall.h>
#include <sched.h>
#include <time.h>
#include <algorithm>
#include <unistd.h>
#include <iostream>

/* perf_event_open syscall wrapper */
static long
sys_perf_event_open(struct perf_event_attr *hw_event,
                    pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

static inline pid_t gettid()
{
    return syscall(SYS_gettid);
}

void write_linear(std::vector<int> &v)
{
    for (uint j = 0; j < v.size(); j++) {
        v[j] = j;
    }
}

void write_random(std::vector<int> &v)
{
    write_linear(v);
    std::random_shuffle(v.begin(), v.end());
}


void read_linear(std::vector<int> &v)
{
    volatile int c;
    for (uint j = 0; j < v.size(); j++) {
        c = v[j];
        (void) c;
    }
}

void read_random(std::vector<int> &v, std::vector<int> &vRand)
{
    volatile int b;
    int size = v.size();
    for (uint i = 0; i < (1 << 20); i++) {
        b = v[vRand[i % size]];
        (void) b;
    }
}

double experiment(qint64 len, int fdRef, int fdMiss)
{
    std::vector<int> v(len);
    std::vector<int> vRand(len);
    int samples = 10;
    double rate = 0.0;
    for (int i = 0; i < samples; i++) {
        long vRefI, vMissI, vRefF, vMissF;
        int ret = 0;

        write_linear(v);
        write_random(vRand);
        read_linear(v);

        ret = read(fdRef, &vRefI, sizeof(vRefI));
        ret = read(fdMiss, &vMissI, sizeof(vMissI));

        read_random(v, vRand);

        ret = read(fdRef, &vRefF, sizeof(vRefF));
        ret = read(fdMiss, &vMissF, sizeof(vMissF));

        vMissF -= vMissI;
        vRefF -= vRefI;
        double delta = (double)vMissF / vRefF;
        qDebug() << "ret" << ret << " L1 Cache Accesses: "<< vRefF << " L1 Cache Misses: "<< vMissF <<" value " << delta;
        rate += delta;
    }
    return rate / samples;
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    perf_event_attr attr, attr2;

    // Execution sur un seul CPU
    pid_t tid = gettid();
    cpu_set_t  mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    qDebug() << sched_setaffinity(0, sizeof(mask), &mask);

    //(perf_hw_cache_id) | (perf_hw_cache_op_id << 8) |
    //(perf_hw_cache_op_result_id << 16)

    // premiere structure d'attributs
    memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(attr);
    attr.type = PERF_TYPE_HW_CACHE;
    attr.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
    attr.inherit = 1;
    attr.disabled = 0;

    // Deuxieme structure
    memset(&attr2, 0, sizeof(attr2));
    attr2.size = sizeof(attr2);
    attr2.type = PERF_TYPE_HW_CACHE;
    attr2.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
    attr2.inherit = 1;
    attr2.disabled = 0;

    //pid == 0 and cpu == -1
    //This measures the calling process/thread on any CPU.
    int fdRef = sys_perf_event_open(&attr, tid, -1, -1, 0);
    int fdMiss = sys_perf_event_open(&attr2, tid, -1, -1, 0);
    //int fdMiss = sys_perf_event_open(&attr2, tid, -1, fdRef, 0);

    QMap<int, double> results;
    for (qint64 len = 1; len < (1 << 26); len *= 2) {
        double miss_rate = experiment(len, fdRef, fdMiss);
        results.insert(len * sizeof(int), miss_rate);
    }

    // report
    std::cout << "size,miss_rate" << std::endl;
    for (int size : results.keys()) {
        std::cout << size << "," << results[size] << std::endl;
    }

    close(fdRef);
    close(fdMiss);
    return 0;
}
