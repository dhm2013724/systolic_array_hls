
#include "systolic_array.h"

typedef struct {
	int K_MIN;
	bool write_back;
	bool init;
} cwrap_s;

template<int MIN_R,int MID_C>
void compute_pe(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out,
		hls::stream<cwrap_s> &cws_in, hls::stream<cwrap_s> &cws_out)
{
	cwrap_s cws;
	cws_in >> cws;
	int K = cws.K_MIN;
	bool write_back = cws.write_back;
	bool init = cws.init;
	cws_out << cws;

#pragma HLS INLINE off
	static int C_local;
	int A_tmp, B_tmp;

	if(init){
		C_local = 0;
	}

	for(int k=0; k<K; k++)
	{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
#pragma HLS PIPELINE II=1
		A_in >> A_tmp;
		B_in >> B_tmp;

		C_local += A_tmp * B_tmp;
		A_out << A_tmp;
		B_out << B_tmp;
	}

	if(write_back){
		C_out.write(C_local);
		//tranfer neighbor PE's output
		for(int k=0; k<MID_C; k++){
#pragma HLS PIPELINE II=1
			int tmp_in, tmp_out;
			C_in  >> tmp_in;
			tmp_out = tmp_in;
			C_out << tmp_out;
		}
	}

	return;
}

void Drain(hls::stream<int> &in, int data_num)
{
	int drain;
	for(int k = 0; k<data_num; k++){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
#pragma HLS PIPELINE II=1
		in >> drain;
	}
}

void Load_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N],
		hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
		hls::stream<cwrap_s> cws_s[TILE_M], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX){

	static int K_cnt;
	if(init){
		K_cnt = 0;
	}
	K_cnt += K_MIN;

	for(int k=0; k<K_MIN; k++)
	{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
#pragma HLS PIPELINE II=1
		for(int i=0; i<TILE_M; i++){
			int tmp = 0;
			if(i<M_MIN)
				tmp = A_local[i][k];
			Ain_s[i] << tmp;
		}

		for(int j=0; j<TILE_N; j++){
			int tmp = 0;
			if(j<N_MIN)
				tmp = B_local[k][j];
			Bin_s[j] << tmp;
		}
	}

	bool wb = (K_cnt == K_MAX);

	for(int i=0; i<TILE_M; i++){
#pragma HLS UNROLL
		cwrap_s cws;
		cws.K_MIN = K_MIN;
		cws.write_back = wb;
		cws.init = init;

		cws_s[i] << cws;
	}
}

void Compute_wrapper(hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
		hls::stream<int> Cout_s[TILE_M], hls::stream<cwrap_s> cws_s[TILE_M], hls::stream<bool> &wb1){

#pragma HLS DATAFLOW

	hls::stream<int> A_inter[TILE_M][TILE_N+1];
#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_K
#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_K
	hls::stream<int> B_inter[TILE_M+1][TILE_N];
#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_K
#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_K
	hls::stream<int> C_out[TILE_M][TILE_N+1];
#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_TILE_N
#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_TILE_N

	hls::stream<cwrap_s> cws_inter[TILE_M][TILE_N+1];
#pragma HLS STREAM variable=cws_inter dim=1 depth=2
#pragma HLS STREAM variable=cws_inter dim=2 depth=2

#SA_GEN#

	DRAIN_AB:{
		cwrap_s cws;
		cws_inter[0][TILE_N] >> cws;

		for(int i=1; i<TILE_M; i++)
		{
#pragma HLS UNROLL
			cwrap_s cws_t;
			cws_inter[i][TILE_N] >> cws_t;
		}

		for(int i=0; i<TILE_M; i++)
		{
#pragma HLS UNROLL
			Drain(A_inter[i][TILE_N], cws.K_MIN);
		}

		for(int j=0; j<TILE_N; j++)
		{
#pragma HLS UNROLL
			Drain(B_inter[TILE_M][j], cws.K_MIN);
		}

		wb1.write(cws.write_back);
	}
}

void Write_wrapper(int C_local[TILE_M][TILE_N], hls::stream<int> Cout_s[TILE_M], int M_MIN, int N_MIN, int K_MIN, hls::stream<bool> &wb1)
{
	bool write_back = wb1.read();
	if(write_back){
		for(int j=0; j<TILE_N; j++){
#pragma HLS PIPELINE II=1
			for(int i=0; i<TILE_M; i++)
			{
				int tmp_out;
				Cout_s[i] >> tmp_out;
				C_local[i][TILE_N-1-j] = tmp_out;
			}
		}
	}
}

void Compute_SA(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int C_local[TILE_M][TILE_N], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
{

#pragma HLS DATAFLOW

	hls::stream<int> Ain_s[TILE_M];
#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_K
	hls::stream<int> Bin_s[TILE_N];
#pragma HLS STREAM variable=Bin_s depth=DEPTH_TILE_K
	hls::stream<int> Cout_s[TILE_M];
#pragma HLS STREAM variable=Cout_s depth=DEPTH_TILE_N
	hls::stream<cwrap_s> cws_s[TILE_M];
#pragma HLS STREAM variable=cws_s
	hls::stream<bool> wb1;

	Load_wrapper( A_local, B_local, Ain_s, Bin_s, cws_s, init, M_MIN, N_MIN, K_MIN, K_MAX);

	Compute_wrapper( Ain_s, Bin_s, Cout_s, cws_s, wb1);

	Write_wrapper( C_local, Cout_s, M_MIN, N_MIN, K_MIN, wb1);

}

void Load_A(int A_local[TILE_M][TILE_K], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
{
	int base_offset = M_base*K_len + K_base;
	Loop_M:for(int i=0; i<M_MIN; i++)
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
			Loop_K:for(int k=0; k<K_MIN; k++)
			{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
#pragma HLS PIPELINE II=1
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
#pragma HLS PIPELINE II=1
				B_local[i][k] = B[base_offset + i*N_len + k];
			}
}

void Load_AB_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int *A, int *B,
		int K, int N, int i, int j, bool k_init, int M_MIN, int N_MIN, int K_MIN0[1], int k0[1])
{

	static int k;
	if(k_init){
		k = 0;
	}else{
		k += TILE_K;
	}

	int K_MIN = MIN(TILE_K, K-k);

	Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
	Load_B(B_local, B, k, j, N, K_MIN, N_MIN);

	K_MIN0[0] = K_MIN;
	k0[0] = k;
}

void Store_C(int C_local[TILE_M][TILE_N], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
{
	int base_offset = M_base*N_len + N_base;
	Loop_K:for(int i=0; i<M_MIN; i++)
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
			Loop_N:for(int k=0; k<N_MIN; k++)
			{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
#pragma HLS PIPELINE II=1
				C[base_offset + i*N_len + k] = C_local[i][k];
			}
}

void DATAFLOW_Load_Compute(int C_local[TILE_M][TILE_N], int *A, int *B, int M, int N, int K, int kloops, int i, int j, int M_MIN, int N_MIN)
{
	static int A_local[TILE_M][TILE_K];
#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
	static int B_local[TILE_K][TILE_N];
#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2

	int K_MIN0[1], k0[1];

	Loop_K:for(int k=0; k<kloops; k++)
	{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
#pragma HLS DATAFLOW
		Load_AB_wrapper( A_local, B_local, A, B, K, N, i, j, k==0, M_MIN, N_MIN, K_MIN0, k0);

		Compute_SA(A_local, B_local, C_local, k0[0]==0, M_MIN, N_MIN, K_MIN0[0], K);
	}
}

void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops)//A[MxK]*B[KxN]=C[MxN]
{
#pragma HLS INTERFACE m_axi depth=384 port=A offset=slave bundle=DB_A
#pragma HLS INTERFACE m_axi depth=768 port=B offset=slave bundle=DB_B
#pragma HLS INTERFACE m_axi depth=2048 port=C offset=slave bundle=DB_C

#pragma HLS INTERFACE s_axilite register port=return bundle=CB
#pragma HLS INTERFACE s_axilite register port=M bundle=CB
#pragma HLS INTERFACE s_axilite register port=N bundle=CB
#pragma HLS INTERFACE s_axilite register port=K bundle=CB
#pragma HLS INTERFACE s_axilite register port=kloops bundle=CB

#pragma HLS INTERFACE s_axilite register port=A bundle=CB
#pragma HLS INTERFACE s_axilite register port=B bundle=CB
#pragma HLS INTERFACE s_axilite register port=C bundle=CB

	static int C_local[TILE_M][TILE_N];
#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1

	int M_MIN, N_MIN;

	Loop_M:for(int i=0; i<M; i+= TILE_M)
	{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
		M_MIN = MIN(TILE_M, M-i);
		Loop_N:for(int j=0; j<N; j+= TILE_N)
		{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
			N_MIN = MIN(TILE_N, N-j);

			DATAFLOW_Load_Compute( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);

			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
		}
	}
}

