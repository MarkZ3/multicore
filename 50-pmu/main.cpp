#include <QDebug>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cassert>

/* perf_event_open syscall wrapper */
static long
sys_perf_event_open(struct perf_event_attr *hw_event,
                    pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

/* gettid syscall wrapper */
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

namespace CounterTypeEnum {
    enum Type {
        INSTRUCTIONS,
        CYCLES,
        L1_READ,
        L1_MISSES,
    };
    static const Type All[] = {
        INSTRUCTIONS,
        CYCLES,
        L1_READ,
        L1_MISSES,
    };
    static const char *Names[] = {
        "inst",
        "cycles",
        "l1_read",
        "l1_miss",
    };
}


int do_experiment(const CounterTypeEnum::Type type)
{
    perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(attr);

    /*
     * General hardware counter
     * IPC = instruction / cycles
     * Miss ratio = references / misses
     */
    switch (type) {
    case CounterTypeEnum::INSTRUCTIONS:
        attr.type = PERF_TYPE_HARDWARE;
        attr.config = PERF_COUNT_HW_INSTRUCTIONS;
        break;
    case CounterTypeEnum::CYCLES:
        attr.type = PERF_TYPE_HARDWARE;
        attr.config = PERF_COUNT_HW_CPU_CYCLES;
        break;
    case CounterTypeEnum::L1_READ:
        attr.type = PERF_TYPE_HW_CACHE;
        attr.config = (PERF_COUNT_HW_CACHE_L1D) |
                (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
        break;
    case CounterTypeEnum::L1_MISSES:
        attr.type = PERF_TYPE_HW_CACHE;
        attr.config = (PERF_COUNT_HW_CACHE_L1D) |
                (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
        break;
    default:
        qDebug() << "invalid counter selected";
        return -1;
    }

    pid_t tid = gettid();
    int fd = sys_perf_event_open(&attr, tid, -1, -1, 0);

    int samples = 10;
    long sum = 0;

    int len = 1000;
    std::vector<int> v(len);
    std::vector<int> buf(1E6);
    (void) buf;

    for (int i = 0; i < samples; i++) {
        long v1, v2;
        int ret = 0;

        // Read the counter value
        ret = read(fd, &v1, sizeof(v1));
        assert(ret > 0);

        // do some work
        write_linear(v);

        // Read the counter value
        ret = read(fd, &v2, sizeof(v2));
        assert(ret > 0);

        long delta = v2 - v1;
        qDebug() << CounterTypeEnum::Names[type] << "value" << delta;
        sum += delta;
    }
    qDebug() << CounterTypeEnum::Names[type] << " avg value" << sum / samples;
    close(fd);
    return 0;
}

int main()
{
    for (const auto type : CounterTypeEnum::All) {
        do_experiment(type);
    }
    return 0;
}
