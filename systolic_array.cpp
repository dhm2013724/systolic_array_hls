
#include "systolic_array.h"

#define TILE_M 2
#define TILE_N 2
#define TILE_K 2

//////////////////////////////////////////v3 start////////////////////////////////////////
void Load_A(int *A_local, int *A, int M_base, int K_base, int K_len)
{
	int dst_offset = 0;
	int base_offset = M_base*K_len + K_base;
	Loop_M:for(int i=0; i<TILE_M; i++)
			Loop_K:for(int k=0; k<TILE_K; k++)
			{
				A_local[dst_offset] = A[base_offset + i*K_len + k];
				dst_offset++;
			}
}

void Load_B(int *B_local, int *B, int K_base, int N_base, int N_len)
{
	int dst_offset = 0;
	int base_offset = K_base*N_len + N_base;
	Loop_K:for(int i=0; i<TILE_K; i++)
			Loop_N:for(int k=0; k<TILE_N; k++)
			{
				B_local[dst_offset] = B[base_offset + i*N_len + k];
				dst_offset++;
			}
}

void Compute(int *A_local, int *B_local, int *C_local, bool init)
{
	Loop_M:for(int i=0; i<TILE_M; i++)
		Loop_N:for(int j=0; j<TILE_N; j++)
			Loop_K:for(int k=0; k<TILE_K; k++)
			{
				int tmp;
				if(init&&k==0)
					tmp = 0;
				else
					tmp = C_local[i*TILE_N + j];

				C_local[i*TILE_N + j] = tmp + A_local[i*TILE_K + k] * B_local[k*TILE_N + j];
			}
}

void Store_C(int *C_local, int *C, int M_base, int N_base, int N_len)
{
	int src_offset = 0;
	int base_offset = M_base*N_len + N_base;
	Loop_K:for(int i=0; i<TILE_M; i++)
			Loop_N:for(int k=0; k<TILE_N; k++)
			{
				C[base_offset + i*N_len + k] = C_local[src_offset];
				src_offset++;
			}
}

void MUL(int *A, int *B, int *C, int M, int N, int K)//A[MxK]*B[KxN]=C[MxN]
{

#pragma HLS INTERFACE m_axi depth=65535 port=A offset=slave bundle=DB_A
#pragma HLS INTERFACE m_axi depth=65535 port=B offset=slave bundle=DB_B
#pragma HLS INTERFACE m_axi depth=65535 port=C offset=slave bundle=DB_C

#pragma HLS INTERFACE s_axilite register port=return bundle=CB
#pragma HLS INTERFACE s_axilite register port=M bundle=CB
#pragma HLS INTERFACE s_axilite register port=N bundle=CB
#pragma HLS INTERFACE s_axilite register port=K bundle=CB

#pragma HLS INTERFACE s_axilite register port=A bundle=CB
#pragma HLS INTERFACE s_axilite register port=B bundle=CB
#pragma HLS INTERFACE s_axilite register port=C bundle=CB

	static int A_local[TILE_M*TILE_K];
	static int B_local[TILE_K*TILE_N];
	static int C_local[TILE_M*TILE_N];

	Loop_M:for(int i=0; i<M; i+= TILE_M)
		Loop_N:for(int j=0; j<N; j+= TILE_N)
		{
			Loop_K:for(int k=0; k<K; k+= TILE_K)
			{
				Load_A(A_local, A, i, k, K);
				Load_B(B_local, B, k, j, N);

				Compute(A_local, B_local, C_local, k==0);
			}

			Store_C(C_local, C, i, j, N);
		}
}
//////////////////////////////////////////v3 end////////////////////////////////////////

//////////////////////////////////////////v2 start////////////////////////////////////////
//void MUL(int *A, int *B, int *C, int M, int N, int K)//A[MxK]*B[KxN]=C[MxN]
//{
//
//#pragma HLS INTERFACE m_axi depth=65535 port=A offset=slave bundle=DB_A
//#pragma HLS INTERFACE m_axi depth=65535 port=B offset=slave bundle=DB_B
//#pragma HLS INTERFACE m_axi depth=65535 port=C offset=slave bundle=DB_C
//
//#pragma HLS INTERFACE s_axilite register port=return bundle=CB
//#pragma HLS INTERFACE s_axilite register port=M bundle=CB
//#pragma HLS INTERFACE s_axilite register port=N bundle=CB
//#pragma HLS INTERFACE s_axilite register port=K bundle=CB
//
//#pragma HLS INTERFACE s_axilite register port=A bundle=CB
//#pragma HLS INTERFACE s_axilite register port=B bundle=CB
//#pragma HLS INTERFACE s_axilite register port=C bundle=CB
//
//	Loop_M:for(int i=0; i<M; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
//		Loop_N:for(int j=0; j<N; j++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
//			Loop_K:for(int k=0; k<K; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
//
//				int tmp;
//				if(k==0)
//					tmp = 0;
//				else
//					tmp = C[i*N + j];
//
//				C[i*N + j] = tmp + A[i*K + k]*B[k*N + j];
//			}
//}
//////////////////////////////////////////v2 end////////////////////////////////////////

//////////////////////////////////////////v1 start////////////////////////////////////////
//void MUL(int *A, int *B, int *C, int M, int N, int K)//A[MxK]*B[KxN]=C[MxN]
//{
//	for(int i=0; i<M; i++)
//		for(int j=0; j<N; j++)
//			for(int k=0; k<K; k++)
//			{
//				int tmp;
//				if(k==0)
//					tmp = 0;
//				else
//					tmp = C[i*N + j];
//
//				C[i*N + j] = tmp + A[i*K + k]*B[k*N + j];
//			}
//}
//////////////////////////////////////////v1 end////////////////////////////////////////


