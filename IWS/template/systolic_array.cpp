
#include "systolic_array.h"

typedef struct {
	bool wb;
	bool init;
} wb_wrap_s;

class PE_cls {
protected:
	int weight_local;
	int tmp_buf[TILE_M];
public:
template<int MIN_R,int MID_C>
	void compute(hls::stream<int> &A_in, hls::stream<int> &M_MIN_s_in, hls::stream<int> &C_in,
			hls::stream<int> &A_out, hls::stream<int> &M_MIN_s_out, hls::stream<int> &C_out);
template<int MIN_R,int MID_C>
	void fillB(hls::stream<int> &B_in, hls::stream<int> &B_out);
};

template<int MIN_R,int MID_C>
void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &M_MIN_s_in, hls::stream<int> &C_in,
		hls::stream<int> &A_out, hls::stream<int> &M_MIN_s_out, hls::stream<int> &C_out)
{
	const int DEPTH_COUT_S_single = (MIN_R+1)*TILE_M;
#pragma HLS STREAM variable=C_out depth=DEPTH_COUT_S_single
#pragma HLS INLINE off

	int M_MIN;
	M_MIN_s_in >> M_MIN;
	M_MIN_s_out << M_MIN;

	for(int i = 0; i < M_MIN; i++){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
#pragma HLS PIPELINE II=1
		int tmp, A_tmp;
		A_in >> A_tmp;
		tmp_buf[i] = A_tmp * weight_local;
		A_out << A_tmp;
	}

	for(int i = 0; i < M_MIN; i++){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
#pragma HLS PIPELINE II=1
		C_out << tmp_buf[i];
	}

	int trans_num = MIN_R*M_MIN;

	for(int i = 0; i < trans_num; i++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=PE_TRANS_NUM_MAX
#pragma HLS PIPELINE II=1
		int tmp;
		C_in >> tmp;
		C_out << tmp;
	}

	return;
}

template<int MIN_R,int MID_C>
void PE_cls::fillB(hls::stream<int> &B_in, hls::stream<int> &B_out){
#pragma HLS INLINE off
	int B_tmp;
	B_in >> B_tmp;
	weight_local = B_tmp;
//tranfer neighbor PE's weight
	for(int k=0; k<SA_R-1-MIN_R; k++){
#pragma HLS PIPELINE II=1
		int tmp_in, tmp_out;
		B_in  >> tmp_in;
		tmp_out = tmp_in;
		B_out << tmp_out;
	}
}

void Drain(hls::stream<int> &in, int data_num)
{
#pragma HLS INLINE off
	int drain;
	for(int k = 0; k<data_num; k++){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
#pragma HLS PIPELINE II=1
		in >> drain;
	}
}

void Writeback_SA_Cout(int C_local[TILE_M][SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb1_s)
{
	static int partial_sum[TILE_M][SA_C];
#pragma HLS ARRAY_PARTITION variable=partial_sum complete dim=2
	wb_wrap_s wbw_s;

	wbw_s = wb1_s.read();
	bool init = wbw_s.init;
	bool wb = wbw_s.wb;

	int k = 0;
	while(k<SA_R){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
		L2:for(int i=0; i<M_MIN;i++)
		{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
#pragma HLS PIPELINE II=1
//#pragma HLS DEPENDENCE variable=partial_sum inter false
			for(int j=0; j<SA_C; j++)
			{
				int tmp, psum;
				if((k==0)&&init)
					psum = 0;
				else
					psum = partial_sum[i][j];
				tmp = Cout_s[j].read();
				partial_sum[i][j] = psum + tmp;
			}
		}
		k++;
	}

	if(wb){
		for(int i=0; i<M_MIN; i++){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
				for(int j=0; j<N_MIN; j++){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
#pragma HLS PIPELINE II=1
			C_local[i][j] = partial_sum[i][j];
				}
		}
	}
}

void Compute_SA(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], hls::stream<int> Cout_s[SA_C], hls::stream<int> M_MIN_s[SA_R],
		hls::stream<wb_wrap_s> &wb0_s, hls::stream<wb_wrap_s> &wb1_s)
{
#pragma HLS DATAFLOW

	hls::stream<int> A_inter[SA_R][SA_C+1];
#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_M
#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_M
	hls::stream<int> B_inter[SA_R+1][SA_C];
#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_SA_R
#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_SA_R
	hls::stream<int> C_out[SA_R+1][SA_C];

	hls::stream<int> M_MIN_inter[SA_R][SA_C+1];
#pragma HLS STREAM variable=M_MIN_inter dim=1
#pragma HLS STREAM variable=M_MIN_inter dim=2

	static PE_cls PE_array[SA_R][SA_C];
#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=1
#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=2

#FILL_B#

#Compute_SA_Loop#

	DRAIN_MIN_M:{
		for(int i=1; i<SA_R; i++)
		{
#pragma HLS UNROLL
			int tmp;
			M_MIN_inter[i][SA_C] >> tmp;
//			Drain(M_MIN_inter[i][SA_C], M_MIN);
		}
	}

	int M_MIN;
	M_MIN_inter[0][SA_C] >> M_MIN;

	DRAIN_A:for(int i=0; i<SA_R; i++)
	{
#pragma HLS UNROLL
		Drain(A_inter[i][SA_C], M_MIN);
	}

	wb_wrap_s wbw_s;
	wbw_s = wb0_s.read();
	wb1_s.write(wbw_s);
}

void Load_A(hls::stream<int> Ain_s[SA_R], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
{
	int base_offset = M_base*K_len + K_base;
	Loop_M:for(int i=0; i<M_MIN; i++)
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
			Loop_K:for(int k=0; k<SA_R; k++)
			{
#pragma HLS PIPELINE II=1
				int tmp_in;
				if(k<K_MIN)
					tmp_in = A[base_offset + i*K_len + k];
				else
					tmp_in = 0;
				Ain_s[k] << tmp_in;
			}
}

void Load_B(hls::stream<int> Bin_s[SA_C], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
{
	int base_offset = K_base*N_len + N_base;
	Loop_K:for(int i=0; i<SA_R; i++)
			Loop_N:for(int k=0; k<SA_C; k++)
			{
#pragma HLS PIPELINE II=1
				int tmp_in;
				bool k_en = i<K_MIN;
				bool n_en = k<N_MIN;
				bool ld_en = k_en && n_en;
				if(ld_en)
					tmp_in = B[base_offset + i*N_len + k];
				else
					tmp_in = 0;
				Bin_s[k] << tmp_in;
			}
}

void Store_C(int C_local[TILE_M][SA_C], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
{
	int base_offset = M_base*N_len + N_base;
	Loop_K:for(int i=0; i<M_MIN; i++)
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
			Loop_N:for(int k=0; k<N_MIN; k++)
			{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
#pragma HLS PIPELINE II=1
				C[base_offset + i*N_len + k] = C_local[i][k];
			}
}

void Trans_M_MIN(hls::stream<int> M_MIN_s[SA_R], int M_MIN){

#pragma HLS INLINE off

	for(int i = 0;i<SA_R;i++){
#pragma HLS UNROLL
		M_MIN_s[i] << M_MIN;
	}

}

void Load_AB_wrapper(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], int *A, int *B, int i, int j, bool k_init,
		int K, int N, int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb0_s, hls::stream<int> M_MIN_s[SA_R]){

	static int k;
	if(k_init){
		k = 0;
	}else{
		k += SA_R;
	}

	int K_MIN = MIN(SA_R, K-k);

	Load_A(Ain_s, A, i, k, K, M_MIN, K_MIN);
	Load_B(Bin_s, B, k, j, N, K_MIN, N_MIN);
	Trans_M_MIN(M_MIN_s, M_MIN);

	wb_wrap_s wbw_s0;
	wbw_s0.wb = ((k+K_MIN) == K);
	wbw_s0.init = (k == 0);
	wb0_s.write(wbw_s0);
}

void DATAFLOW_SA_LCW(int C_local[TILE_M][SA_C], int *A, int *B, int M, int N, int K, int kloops,
		int i, int j, int M_MIN, int N_MIN)
{
	hls::stream<int> Ain_s[SA_R];
#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_M
	hls::stream<int> Bin_s[SA_C];
#pragma HLS STREAM variable=Bin_s depth=DEPTH_SA_R
	hls::stream<int> Cout_s[SA_C];
#pragma HLS STREAM variable=Cout_s depth=DEPTH_COUT_S

	hls::stream<wb_wrap_s> wb0_s;
	hls::stream<wb_wrap_s> wb1_s;

	hls::stream<int> M_MIN_s[SA_R];

	Loop_K:for(int k=0; k<kloops; k++)
	{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
#pragma HLS DATAFLOW
		Load_AB_wrapper( Ain_s, Bin_s, A, B, i, j, k==0, K, N, M_MIN, N_MIN, wb0_s, M_MIN_s);

		Compute_SA(Ain_s, Bin_s, Cout_s, M_MIN_s, wb0_s, wb1_s);

		Writeback_SA_Cout( C_local, Cout_s, M_MIN, N_MIN, wb1_s);
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

	static int C_local[TILE_M][SA_C];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1

	int M_MIN, N_MIN;
	Loop_M:for(int i=0; i<M; i+= TILE_M)
	{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
		M_MIN = MIN(TILE_M, M-i);
		Loop_N:for(int j=0; j<N; j+= SA_C)
		{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
			N_MIN = MIN(SA_C, N-j);
			DATAFLOW_SA_LCW( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);

			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
		}
	}
}

