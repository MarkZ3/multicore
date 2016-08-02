#include <QDebug>
#include <QString>
#include <cstdio>
#include <cassert>

#include "ops.h"
#include "vec.h"
#include <xmmintrin.h>

__attribute__((noinline))
float fadd_cpp(float x, float y)
{
    return x + y;
}

__attribute__((noinline))
void array_addss_cpp(float *vector, float scalar, int length)
{
    for (int i = 0; i < length; i++) {
        vector[i] = vector[i] + scalar;
    }
}

__attribute__((noinline))
void array_addps_cpp_vect(float *vector, float scalar, int length)
{
    __m128 s = _mm_set_ps(scalar, scalar, scalar, scalar);

    int chunks = length / 4;
    int round_down = chunks * 4;
    for (int i = 0; i < round_down; i+=4) {
        __m128 d = _mm_loadu_ps(vector + i);
        __m128 a = _mm_add_ps(d, s);
        _mm_storeu_ps(vector + i, a);
    }

    // tail of the vector, if any
    for (int i = round_down; i < length; i++) {
        vector[i] = vector[i] + scalar;
    }

}

void assembly_ops()
{
    hello();

    std::printf("%lX\n", add2(1, 2));

    long cafebabe1 = add10_v1(0xCAFE0000, 0, 0, 0, 0, 0, 0, 0, 0, 0x0000BABE);
    long cafebabe2 = add10_v2(0xCAFE0000, 0, 0, 0, 0, 0, 0, 0, 0, 0x0000BABE);
    std::printf("%lX\n", cafebabe1);
    std::printf("%lX\n", cafebabe2);

    std::printf("%ld\n", min3_v1(3, 10, -12));
    std::printf("%ld\n", max3_v1(3, 10, -12));

    std::printf("%ld\n", eq2(1, 2));
    std::printf("%ld\n", eq2(2, 2));

    std::printf("%f\n", fadd_cpp(1.1, 1.1));
    std::printf("%f\n", fadd2(1.1, 1.1));
}

void reset(QVector<float> &vec)
{
    for (int i = 0; i < vec.size(); i++)
        vec[i] = 0.0;
}

#define ROUND_DOWN(x, s) ((x) & ~((s)-1))
void assembly_vec()
{
    long n = 10;
    QVector<float> data(n);
    float val = -8346975.0; // 0xcafebabe

    movss_ex1();
    movups_ex1();

    reset(data);
    qDebug() << "orig" << data;

    reset(data);
    array_addss_cpp(data.data(), val, data.size());
    qDebug() << "cpp " << data;

    reset(data);
    array_addss_iter(data.data(), val, data.size());
    qDebug() << "asmi" << data;

    reset(data);
    array_addps_vect(data.data(), val, data.size());
    qDebug() << "asmv" << data;

    reset(data);
    array_addps_cpp_vect(data.data(), val, data.size());
    qDebug() << "cppv" << data;

}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;
    assembly_ops();
    assembly_vec();

    return 0;
}
