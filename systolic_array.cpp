
#include "systolic_array.h"

//////////////////////////////////////////v7 start////////////////////////////////////////
#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

#define TILE_M 7
#define TILE_N 9
#define TILE_K 5

class PE_cls {
protected:
	int C_local[2];
	int k_cnt;
public:
	int pe_id;
	void compute(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &C_in,
			hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init);
};

void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &C_in,
		hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init)
{
	int A_tmp, B_tmp;
	if(init){
		C_local[0] = 0;
		k_cnt = 0;
	}
	for(int k=0; k<K; k++)
	{
		A_in >> A_tmp;
		B_in >> B_tmp;

		C_local[0] += A_tmp * B_tmp;

		A_out << A_tmp;
		B_out << B_tmp;
		k_cnt++;
	}

	if(k_cnt == K_MAX){
		C_out << C_local[0];
		for(int k=0; k<TILE_N-pe_id-1; k++)
		{
				C_in  >> C_local[1];
				C_out << C_local[1];
		}
	}

	return;
}

void Drain(hls::stream<int> &in, int data_num)
{
	int drain;
	for(int k = 0; k<data_num; k++){
		in >> drain;
	}
}

void Compute_SA(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int C_local[TILE_M][TILE_N], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
{
	hls::stream<int> A_inter[TILE_M][TILE_N+1];
#pragma HLS STREAM variable=A_inter
	hls::stream<int> B_inter[TILE_M+1][TILE_N];
#pragma HLS STREAM variable=B_inter
	hls::stream<int> C_out[TILE_M][TILE_N+1];
#pragma HLS STREAM variable=C_out

	PE_cls PE_array[TILE_M][TILE_N];

	for(int i=0; i<TILE_M; i++)
		for(int k=0; k<K_MIN; k++)
		{
			int tmp = 0;
			if(i<M_MIN)
				tmp = A_local[i][k];
			A_inter[i][0] << tmp;
//			A_inter[i][0] << A_local[i*K_MIN + k];
		}

	for(int j=0; j<TILE_N; j++)
		for(int k=0; k<K_MIN; k++)
		{
			int tmp = 0;
			if(j<N_MIN)
				tmp = B_local[k][j];
			B_inter[0][j] << tmp;
//			B_inter[0][j] << B_local[k*N_MIN + j];
		}

	Loop_M:for(int i=0; i<TILE_M; i++)
		Loop_N:for(int j=0; j<TILE_N; j++)
		{
//			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], &C_local[i][j], K_MIN, K_MAX, init);//, C_out[i][j], K_MIN);
			PE_array[i][j].pe_id = j;
			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], C_out[i][j+1], A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);//, C_out[i][j], K_MIN);
		}

	for(int i=0; i<TILE_M; i++)
	{
		Drain(A_inter[i][TILE_N], K_MIN);
	}

	for(int j=0; j<TILE_N; j++)
	{
		Drain(B_inter[TILE_M][j], K_MIN);
	}

	for(int i=0; i<TILE_M; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
		for(int j=0; j<TILE_N; j++)
		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
#pragma HLS PIPELINE II=1
			int tmp_out;
			C_out[i][0] >> tmp_out;
			if((i < M_MIN) && (j < N_MIN))
				C_local[i][j] = tmp_out;
		}
}

void Load_A(int A_local[TILE_M][TILE_K], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
{
	int base_offset = M_base*K_len + K_base;
	Loop_M:for(int i=0; i<M_MIN; i++)
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
			Loop_K:for(int k=0; k<K_MIN; k++)
			{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)

				A_local[i][k] = A[base_offset + i*K_len + k];
			}
}

void Load_B(int B_local[TILE_K][TILE_N], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
{
	int base_offset = K_base*N_len + N_base;
	Loop_K:for(int i=0; i<K_MIN; i++)
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
			Loop_N:for(int k=0; k<N_MIN; k++)
			{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)

				B_local[i][k] = B[base_offset + i*N_len + k];
			}
}

void Store_C(int C_local[TILE_M][TILE_N], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
{
	int base_offset = M_base*N_len + N_base;
	Loop_K:for(int i=0; i<M_MIN; i++)
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
			Loop_N:for(int k=0; k<N_MIN; k++)
			{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
				C[base_offset + i*N_len + k] = C_local[i][k];
			}
}

void MUL(int *A, int *B, int *C, int M, int N, int K)//A[MxK]*B[KxN]=C[MxN]
{

#pragma HLS INTERFACE m_axi depth=384 port=A offset=slave bundle=DB_A
#pragma HLS INTERFACE m_axi depth=768 port=B offset=slave bundle=DB_B
#pragma HLS INTERFACE m_axi depth=2048 port=C offset=slave bundle=DB_C

#pragma HLS INTERFACE s_axilite register port=return bundle=CB
#pragma HLS INTERFACE s_axilite register port=M bundle=CB
#pragma HLS INTERFACE s_axilite register port=N bundle=CB
#pragma HLS INTERFACE s_axilite register port=K bundle=CB

#pragma HLS INTERFACE s_axilite register port=A bundle=CB
#pragma HLS INTERFACE s_axilite register port=B bundle=CB
#pragma HLS INTERFACE s_axilite register port=C bundle=CB

	static int A_local[TILE_M][TILE_K];
	static int B_local[TILE_K][TILE_N];
	static int C_local[TILE_M][TILE_N];

	int M_MIN, N_MIN, K_MIN;

	Loop_M:for(int i=0; i<M; i+= TILE_M)
	{
		M_MIN = MIN(TILE_M, M-i);
		Loop_N:for(int j=0; j<N; j+= TILE_N)
		{
			N_MIN = MIN(TILE_N, N-j);
			Loop_K:for(int k=0; k<K; k+= TILE_K)
			{
				K_MIN = MIN(TILE_K, K-k);

				Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
				Load_B(B_local, B, k, j, N, K_MIN, N_MIN);

				Compute_SA(A_local, B_local, C_local, k==0, M_MIN, N_MIN, K_MIN, K);
			}

			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
		}
	}
}
//////////////////////////////////////////v7 end////////////////////////////////////////

////////////////////////////////////////////v6 start////////////////////////////////////////
//#define MAX(x,y) ((x)>(y)?(x):(y))
//#define MIN(x,y) ((x)<(y)?(x):(y))
//
//#define TILE_M 7
//#define TILE_N 9
//#define TILE_K 5
//
//class PE_cls {
//protected:
//	int C_local;
////	int k_cnt;
//public:
//	int pe_id;
//	void compute(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &A_out, hls::stream<int> &B_out, int *C_out, int K, int K_MAX, bool init);
//};
//
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &A_out, hls::stream<int> &B_out, int *C_out, int K, int K_MAX, bool init)
//{
//	int A_tmp, B_tmp;
//	if(init){
//		C_local = 0;
////		k_cnt = 0;
//	}
//	for(int k=0; k<K; k++)
//	{
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//
//		A_out << A_tmp;
//		B_out << B_tmp;
////		k_cnt++;
//	}
////	if(k_cnt == K_MAX)
//		*C_out = C_local;
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//		in >> drain;
//	}
//}
//
//void Compute_SA(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int C_local[TILE_M][TILE_N], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
//{
//	hls::stream<int> A_inter[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=A_inter
//	hls::stream<int> B_inter[TILE_M+1][TILE_N];
//#pragma HLS STREAM variable=B_inter
////	hls::stream<int> C_out[TILE_M][TILE_N];
////#pragma HLS STREAM variable=C_out
//
//	PE_cls PE_array[TILE_M][TILE_N];
//
//	for(int i=0; i<TILE_M; i++)
//		for(int k=0; k<K_MIN; k++)
//		{
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i][k];
//			A_inter[i][0] << tmp;
////			A_inter[i][0] << A_local[i*K_MIN + k];
//		}
//
//	for(int j=0; j<TILE_N; j++)
//		for(int k=0; k<K_MIN; k++)
//		{
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			B_inter[0][j] << tmp;
////			B_inter[0][j] << B_local[k*N_MIN + j];
//		}
//
//	Loop_M:for(int i=0; i<TILE_M; i++)
//		Loop_N:for(int j=0; j<TILE_N; j++)
//		{
//			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], &C_local[i][j], K_MIN, K_MAX, init);//, C_out[i][j], K_MIN);
//		}
//
//	for(int i=0; i<TILE_M; i++)
//	{
//		Drain(A_inter[i][TILE_N], K_MIN);
//	}
//
//	for(int j=0; j<TILE_N; j++)
//	{
//		Drain(B_inter[TILE_M][j], K_MIN);
//	}
//
////	int src_offset = 0;
////	for(int i=0; i<M_MIN; i++)
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
////		for(int j=0; j<N_MIN; j++)
////		{
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
////#pragma HLS PIPELINE II=1
////			C_out[i][j] >> C_local[src_offset];
////			src_offset++;
////		}
//}
//
//void Load_A(int A_local[TILE_M][TILE_K], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<K_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//
//				A_local[i][k] = A[base_offset + i*K_len + k];
//			}
//}
//
//void Load_B(int B_local[TILE_K][TILE_N], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<K_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//
//				B_local[i][k] = B[base_offset + i*N_len + k];
//			}
//}
//
//void Store_C(int C_local[TILE_M][TILE_N], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void MUL(int *A, int *B, int *C, int M, int N, int K)//A[MxK]*B[KxN]=C[MxN]
//{
//
//#pragma HLS INTERFACE m_axi depth=384 port=A offset=slave bundle=DB_A
//#pragma HLS INTERFACE m_axi depth=768 port=B offset=slave bundle=DB_B
//#pragma HLS INTERFACE m_axi depth=2048 port=C offset=slave bundle=DB_C
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
//	static int A_local[TILE_M][TILE_K];
//	static int B_local[TILE_K][TILE_N];
//	static int C_local[TILE_M][TILE_N];
//
//	int M_MIN, N_MIN, K_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//			N_MIN = MIN(TILE_N, N-j);
//			Loop_K:for(int k=0; k<K; k+= TILE_K)
//			{
//				K_MIN = MIN(TILE_K, K-k);
//
//				Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//				Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//				Compute_SA(A_local, B_local, C_local, k==0, M_MIN, N_MIN, K_MIN, K);
//			}
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v6 end////////////////////////////////////////

////////////////////////////////////////////v5 start////////////////////////////////////////
//#define MAX(x,y) ((x)>(y)?(x):(y))
//#define MIN(x,y) ((x)<(y)?(x):(y))
//
//#define TILE_M 4
//#define TILE_N 9
//#define TILE_K 5
//
//void PE(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &A_out, hls::stream<int> &B_out, int *C_out, int K, bool init)//, hls::stream<int> &C_out, int K)
//{
//	int A_tmp, B_tmp;
//	static int C_local;
//	if(init)
//		C_local = 0;
//	else
//		C_local = *C_out;
//	for(int k=0; k<K; k++)
//	{
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//
//		A_out << A_tmp;
//		B_out << B_tmp;
//	}
//	*C_out = C_local;
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//		in >> drain;
//	}
//}
//
//void Compute_SA(int *A_local, int *B_local, int *C_local, bool init, int M_MIN, int N_MIN, int K_MIN)
//{
//	hls::stream<int> A_inter[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=A_inter
//	hls::stream<int> B_inter[TILE_M+1][TILE_N];
//#pragma HLS STREAM variable=B_inter
////	hls::stream<int> C_out[TILE_M][TILE_N];
////#pragma HLS STREAM variable=C_out
//
//	for(int i=0; i<TILE_M; i++)
//		for(int k=0; k<K_MIN; k++)
//		{
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i*K_MIN + k];
//			A_inter[i][0] << tmp;
////			A_inter[i][0] << A_local[i*K_MIN + k];
//		}
//
//	for(int j=0; j<TILE_N; j++)
//		for(int k=0; k<K_MIN; k++)
//		{
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k*N_MIN + j];
//			B_inter[0][j] << tmp;
////			B_inter[0][j] << B_local[k*N_MIN + j];
//		}
//
//	Loop_M:for(int i=0; i<TILE_M; i++)
//		Loop_N:for(int j=0; j<TILE_N; j++)
//		{
//			PE(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], &C_local[i*TILE_N + j], K_MIN, init);//, C_out[i][j], K_MIN);
//		}
//
//	for(int i=0; i<TILE_M; i++)
//	{
//		Drain(A_inter[i][TILE_N], K_MIN);
//	}
//
//	for(int j=0; j<TILE_N; j++)
//	{
//		Drain(B_inter[TILE_M][j], K_MIN);
//	}
//
////	int src_offset = 0;
////	for(int i=0; i<M_MIN; i++)
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
////		for(int j=0; j<N_MIN; j++)
////		{
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
////#pragma HLS PIPELINE II=1
////			C_out[i][j] >> C_local[src_offset];
////			src_offset++;
////		}
//}
//
//void Compute(int *A_local, int *B_local, int *C_local, bool init, int M_MIN, int N_MIN, int K_MIN)
//{
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//		Loop_N:for(int j=0; j<N_MIN; j++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//			Loop_K:for(int k=0; k<K_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//
//				int tmp;
//				if(init&&k==0)
//					tmp = 0;
//				else
//					tmp = C_local[i*N_MIN + j];
//
//				C_local[i*N_MIN + j] = tmp + A_local[i*K_MIN + k] * B_local[k*N_MIN + j];
//			}
//}
//
//void Load_A(int *A_local, int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int dst_offset = 0;
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<K_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//
//				A_local[dst_offset] = A[base_offset + i*K_len + k];
//				dst_offset++;
//			}
//}
//
//void Load_B(int *B_local, int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int dst_offset = 0;
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<K_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//
//				B_local[dst_offset] = B[base_offset + i*N_len + k];
//				dst_offset++;
//			}
//}
//
//void Store_C(int *C_local, int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
////	int src_offset = 0;
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//				C[base_offset + i*N_len + k] = C_local[i*TILE_N + k];
////				C[base_offset + i*N_len + k] = C_local[src_offset];
////				src_offset++;
//			}
//}
//
//void MUL(int *A, int *B, int *C, int M, int N, int K)//A[MxK]*B[KxN]=C[MxN]
//{
//
//#pragma HLS INTERFACE m_axi depth=384 port=A offset=slave bundle=DB_A
//#pragma HLS INTERFACE m_axi depth=768 port=B offset=slave bundle=DB_B
//#pragma HLS INTERFACE m_axi depth=2048 port=C offset=slave bundle=DB_C
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
//	static int A_local[TILE_M*TILE_K];
//	static int B_local[TILE_K*TILE_N];
//	static int C_local[TILE_M*TILE_N];
//
//	int M_MIN, N_MIN, K_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//			N_MIN = MIN(TILE_N, N-j);
//			Loop_K:for(int k=0; k<K; k+= TILE_K)
//			{
//				K_MIN = MIN(TILE_K, K-k);
//
//				Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//				Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//				Compute_SA(A_local, B_local, C_local, k==0, M_MIN, N_MIN, K_MIN);
//			}
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v5 end////////////////////////////////////////

////////////////////////////////////////////v4 start////////////////////////////////////////
//#define MAX(x,y) ((x)>(y)?(x):(y))
//#define MIN(x,y) ((x)<(y)?(x):(y))
//
//#define TILE_M 3
//#define TILE_N 5
//#define TILE_K 7
//
//void Load_A(int *A_local, int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int dst_offset = 0;
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<K_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//
//				A_local[dst_offset] = A[base_offset + i*K_len + k];
//				dst_offset++;
//			}
//}
//
//void Load_B(int *B_local, int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int dst_offset = 0;
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<K_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//
//				B_local[dst_offset] = B[base_offset + i*N_len + k];
//				dst_offset++;
//			}
//}
//
//void Compute(int *A_local, int *B_local, int *C_local, bool init, int M_MIN, int N_MIN, int K_MIN)
//{
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//		Loop_N:for(int j=0; j<N_MIN; j++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//			Loop_K:for(int k=0; k<K_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//
//				int tmp;
//				if(init&&k==0)
//					tmp = 0;
//				else
//					tmp = C_local[i*N_MIN + j];
//
//				C_local[i*N_MIN + j] = tmp + A_local[i*K_MIN + k] * B_local[k*N_MIN + j];
//			}
//}
//
//void Store_C(int *C_local, int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int src_offset = 0;
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//
//				C[base_offset + i*N_len + k] = C_local[src_offset];
//				src_offset++;
//			}
//}
//
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
//	static int A_local[TILE_M*TILE_K];
//	static int B_local[TILE_K*TILE_N];
//	static int C_local[TILE_M*TILE_N];
//
//	int M_MIN, N_MIN, K_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//			N_MIN = MIN(TILE_N, N-j);
//			Loop_K:for(int k=0; k<K; k+= TILE_K)
//			{
//				K_MIN = MIN(TILE_K, K-k);
//
//				Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//				Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//				Compute(A_local, B_local, C_local, k==0, M_MIN, N_MIN, K_MIN);
//			}
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v4 end////////////////////////////////////////

////////////////////////////////////////////v3 start////////////////////////////////////////
//#define MAX(x,y) ((x)>(y)?(x):(y))
//#define MIN(x,y) ((x)<(y)?(x):(y))
//
//#define TILE_M 3
//#define TILE_N 5
//#define TILE_K 7
//
//void Load_A(int *A_local, int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int dst_offset = 0;
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//			Loop_K:for(int k=0; k<K_MIN; k++)
//			{
//				A_local[dst_offset] = A[base_offset + i*K_len + k];
//				dst_offset++;
//			}
//}
//
//void Load_B(int *B_local, int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int dst_offset = 0;
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<K_MIN; i++)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//				B_local[dst_offset] = B[base_offset + i*N_len + k];
//				dst_offset++;
//			}
//}
//
//void Compute(int *A_local, int *B_local, int *C_local, bool init, int M_MIN, int N_MIN, int K_MIN)
//{
//	Loop_M:for(int i=0; i<M_MIN; i++)
//		Loop_N:for(int j=0; j<N_MIN; j++)
//			Loop_K:for(int k=0; k<K_MIN; k++)
//			{
//				int tmp;
//				if(init&&k==0)
//					tmp = 0;
//				else
//					tmp = C_local[i*N_MIN + j];
//
//				C_local[i*N_MIN + j] = tmp + A_local[i*K_MIN + k] * B_local[k*N_MIN + j];
//			}
//}
//
//void Store_C(int *C_local, int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int src_offset = 0;
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//				C[base_offset + i*N_len + k] = C_local[src_offset];
//				src_offset++;
//			}
//}
//
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
//	static int A_local[TILE_M*TILE_K];
//	static int B_local[TILE_K*TILE_N];
//	static int C_local[TILE_M*TILE_N];
//
//	int M_MIN, N_MIN, K_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//			N_MIN = MIN(TILE_N, N-j);
//			Loop_K:for(int k=0; k<K; k+= TILE_K)
//			{
//				K_MIN = MIN(TILE_K, K-k);
//
//				Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//				Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//				Compute(A_local, B_local, C_local, k==0, M_MIN, N_MIN, K_MIN);
//			}
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v3 end////////////////////////////////////////

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


