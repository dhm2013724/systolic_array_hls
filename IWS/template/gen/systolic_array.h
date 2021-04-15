
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

#define TILE_M 5
#define SA_R 13
#define SA_C 11


const int DEPTH_TILE_M = TILE_M;
const int DEPTH_SA_R = SA_R;
const int DEPTH_SA_C = SA_C;
const int DEPTH_COUT_S = SA_R*TILE_M;
const int PE_TRANS_NUM_MAX =  SA_R*TILE_M;

#define NUM_M 32
#define NUM_N 64
#define NUM_K 12

#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)

void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops);//A[MxK]*B[KxN]=C[MxN]

#endif
