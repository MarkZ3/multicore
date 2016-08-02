#ifndef VEC_H
#define VEC_H

#ifdef __cplusplus
extern "C"
{
#endif

void array_addss_iter(float *vector, float scalar, long length);
void array_addps_vect(float *vector, float scalar, long length);
void movss_ex1(void);
void movups_ex1(void);

#ifdef __cplusplus
}
#endif

#endif // VEC_H
