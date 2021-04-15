
#ifndef _H_SYSTOLIC_ARRAY
#define _H_SYSTOLIC_ARRAY

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hls_stream.h>
#include <assert.h>
#include <hls_math.h>

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

#DEFINE_HEADER#

const int DEPTH_TILE_M = TILE_M;
const int DEPTH_TILE_N = TILE_N;
const int DEPTH_TILE_K = TILE_K;

#define NUM_M 32
#define NUM_N 64
#define NUM_K 12

#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)

void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops);//A[MxK]*B[KxN]=C[MxN]

#endif
