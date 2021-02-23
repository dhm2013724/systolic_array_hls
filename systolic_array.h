
#ifndef _H_SYSTOLIC_ARRAY
#define _H_SYSTOLIC_ARRAY

#include <string.h>
#include <hls_stream.h>
#include <assert.h>

#define NUM_M 32
#define NUM_N 64
#define NUM_K 12

#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)

void MUL(int *A, int *B, int *C, int M, int N, int K);//A[MxK]*B[KxN]=C[MxN]

#endif
