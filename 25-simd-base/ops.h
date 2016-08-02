#ifndef OPS_H
#define OPS_H

#ifdef __cplusplus
extern "C"
{
#endif

void hello(void);
long add2(long x, long y);
long add10_v1(long a, long b, long c, long d,
             long e, long f, long g, long h,
             long i, long j);

long add10_v2(long a, long b, long c, long d,
             long e, long f, long g, long h,
             long i, long j);

long min3_v1(long a, long b, long c);
long max3_v1(long a, long b, long c);

long eq2(long x, long y);

float fadd2(float x, float y);

#ifdef __cplusplus
}
#endif

#endif // OPS_H
