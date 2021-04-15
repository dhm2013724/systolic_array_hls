
#include "systolic_array.h"

//////////////////////////////////////////v14 start////////////////////////////////////////

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

//	cwrap_s cws;
//	cws.K_MIN = K_MIN;
//	cws.write_back = wb;
//	cws.init = init;
//	cws0.write(cws);
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

//	cwrap_s cws;
//	cws = cws0.read();

	compute_pe<0, 0>(     Ain_s[0], Bin_s[0], C_out[0][0], A_inter[0][1], B_inter[0+1][0], C_out[0][1],        cws_s[0], cws_inter[0][1]);
	compute_pe<0, 1>(A_inter[0][1], Bin_s[1], C_out[0][1], A_inter[0][2], B_inter[0+1][1], C_out[0][2], cws_inter[0][1], cws_inter[0][2]);
	compute_pe<0, 2>(A_inter[0][2], Bin_s[2], C_out[0][2], A_inter[0][3], B_inter[0+1][2], C_out[0][3], cws_inter[0][2], cws_inter[0][3]);
	compute_pe<0, 3>(A_inter[0][3], Bin_s[3], C_out[0][3], A_inter[0][4], B_inter[0+1][3], C_out[0][4], cws_inter[0][3], cws_inter[0][4]);
	compute_pe<0, 4>(A_inter[0][4], Bin_s[4], C_out[0][4], A_inter[0][5], B_inter[0+1][4], C_out[0][5], cws_inter[0][4], cws_inter[0][5]);
	compute_pe<0, 5>(A_inter[0][5], Bin_s[5], C_out[0][5], A_inter[0][6], B_inter[0+1][5], C_out[0][6], cws_inter[0][5], cws_inter[0][6]);
	compute_pe<0, 6>(A_inter[0][6], Bin_s[6], C_out[0][6], A_inter[0][7], B_inter[0+1][6], C_out[0][7], cws_inter[0][6], cws_inter[0][7]);
	compute_pe<0, 7>(A_inter[0][7], Bin_s[7], C_out[0][7], A_inter[0][8], B_inter[0+1][7], C_out[0][8], cws_inter[0][7], cws_inter[0][8]);
	compute_pe<0, 8>(A_inter[0][8], Bin_s[8], C_out[0][8], A_inter[0][9], B_inter[0+1][8],   Cout_s[0], cws_inter[0][8], cws_inter[0][9]);

	compute_pe<1, 0>(     Ain_s[1], B_inter[1][0], C_out[1][0], A_inter[1][1], B_inter[1+1][0], C_out[1][1],        cws_s[1], cws_inter[1][1]);
	compute_pe<1, 1>(A_inter[1][1], B_inter[1][1], C_out[1][1], A_inter[1][2], B_inter[1+1][1], C_out[1][2], cws_inter[1][1], cws_inter[1][2]);
	compute_pe<1, 2>(A_inter[1][2], B_inter[1][2], C_out[1][2], A_inter[1][3], B_inter[1+1][2], C_out[1][3], cws_inter[1][2], cws_inter[1][3]);
	compute_pe<1, 3>(A_inter[1][3], B_inter[1][3], C_out[1][3], A_inter[1][4], B_inter[1+1][3], C_out[1][4], cws_inter[1][3], cws_inter[1][4]);
	compute_pe<1, 4>(A_inter[1][4], B_inter[1][4], C_out[1][4], A_inter[1][5], B_inter[1+1][4], C_out[1][5], cws_inter[1][4], cws_inter[1][5]);
	compute_pe<1, 5>(A_inter[1][5], B_inter[1][5], C_out[1][5], A_inter[1][6], B_inter[1+1][5], C_out[1][6], cws_inter[1][5], cws_inter[1][6]);
	compute_pe<1, 6>(A_inter[1][6], B_inter[1][6], C_out[1][6], A_inter[1][7], B_inter[1+1][6], C_out[1][7], cws_inter[1][6], cws_inter[1][7]);
	compute_pe<1, 7>(A_inter[1][7], B_inter[1][7], C_out[1][7], A_inter[1][8], B_inter[1+1][7], C_out[1][8], cws_inter[1][7], cws_inter[1][8]);
	compute_pe<1, 8>(A_inter[1][8], B_inter[1][8], C_out[1][8], A_inter[1][9], B_inter[1+1][8],   Cout_s[1], cws_inter[1][8], cws_inter[1][9]);

	compute_pe<2, 0>(     Ain_s[2], B_inter[2][0], C_out[2][0], A_inter[2][1], B_inter[2+1][0], C_out[2][1],        cws_s[2], cws_inter[2][1]);
	compute_pe<2, 1>(A_inter[2][1], B_inter[2][1], C_out[2][1], A_inter[2][2], B_inter[2+1][1], C_out[2][2], cws_inter[2][1], cws_inter[2][2]);
	compute_pe<2, 2>(A_inter[2][2], B_inter[2][2], C_out[2][2], A_inter[2][3], B_inter[2+1][2], C_out[2][3], cws_inter[2][2], cws_inter[2][3]);
	compute_pe<2, 3>(A_inter[2][3], B_inter[2][3], C_out[2][3], A_inter[2][4], B_inter[2+1][3], C_out[2][4], cws_inter[2][3], cws_inter[2][4]);
	compute_pe<2, 4>(A_inter[2][4], B_inter[2][4], C_out[2][4], A_inter[2][5], B_inter[2+1][4], C_out[2][5], cws_inter[2][4], cws_inter[2][5]);
	compute_pe<2, 5>(A_inter[2][5], B_inter[2][5], C_out[2][5], A_inter[2][6], B_inter[2+1][5], C_out[2][6], cws_inter[2][5], cws_inter[2][6]);
	compute_pe<2, 6>(A_inter[2][6], B_inter[2][6], C_out[2][6], A_inter[2][7], B_inter[2+1][6], C_out[2][7], cws_inter[2][6], cws_inter[2][7]);
	compute_pe<2, 7>(A_inter[2][7], B_inter[2][7], C_out[2][7], A_inter[2][8], B_inter[2+1][7], C_out[2][8], cws_inter[2][7], cws_inter[2][8]);
	compute_pe<2, 8>(A_inter[2][8], B_inter[2][8], C_out[2][8], A_inter[2][9], B_inter[2+1][8],   Cout_s[2], cws_inter[2][8], cws_inter[2][9]);

	compute_pe<3, 0>(     Ain_s[3], B_inter[3][0], C_out[3][0], A_inter[3][1], B_inter[3+1][0], C_out[3][1],        cws_s[3], cws_inter[3][1]);
	compute_pe<3, 1>(A_inter[3][1], B_inter[3][1], C_out[3][1], A_inter[3][2], B_inter[3+1][1], C_out[3][2], cws_inter[3][1], cws_inter[3][2]);
	compute_pe<3, 2>(A_inter[3][2], B_inter[3][2], C_out[3][2], A_inter[3][3], B_inter[3+1][2], C_out[3][3], cws_inter[3][2], cws_inter[3][3]);
	compute_pe<3, 3>(A_inter[3][3], B_inter[3][3], C_out[3][3], A_inter[3][4], B_inter[3+1][3], C_out[3][4], cws_inter[3][3], cws_inter[3][4]);
	compute_pe<3, 4>(A_inter[3][4], B_inter[3][4], C_out[3][4], A_inter[3][5], B_inter[3+1][4], C_out[3][5], cws_inter[3][4], cws_inter[3][5]);
	compute_pe<3, 5>(A_inter[3][5], B_inter[3][5], C_out[3][5], A_inter[3][6], B_inter[3+1][5], C_out[3][6], cws_inter[3][5], cws_inter[3][6]);
	compute_pe<3, 6>(A_inter[3][6], B_inter[3][6], C_out[3][6], A_inter[3][7], B_inter[3+1][6], C_out[3][7], cws_inter[3][6], cws_inter[3][7]);
	compute_pe<3, 7>(A_inter[3][7], B_inter[3][7], C_out[3][7], A_inter[3][8], B_inter[3+1][7], C_out[3][8], cws_inter[3][7], cws_inter[3][8]);
	compute_pe<3, 8>(A_inter[3][8], B_inter[3][8], C_out[3][8], A_inter[3][9], B_inter[3+1][8],   Cout_s[3], cws_inter[3][8], cws_inter[3][9]);

	compute_pe<4, 0>(     Ain_s[4], B_inter[4][0], C_out[4][0], A_inter[4][1], B_inter[4+1][0], C_out[4][1],        cws_s[4], cws_inter[4][1]);
	compute_pe<4, 1>(A_inter[4][1], B_inter[4][1], C_out[4][1], A_inter[4][2], B_inter[4+1][1], C_out[4][2], cws_inter[4][1], cws_inter[4][2]);
	compute_pe<4, 2>(A_inter[4][2], B_inter[4][2], C_out[4][2], A_inter[4][3], B_inter[4+1][2], C_out[4][3], cws_inter[4][2], cws_inter[4][3]);
	compute_pe<4, 3>(A_inter[4][3], B_inter[4][3], C_out[4][3], A_inter[4][4], B_inter[4+1][3], C_out[4][4], cws_inter[4][3], cws_inter[4][4]);
	compute_pe<4, 4>(A_inter[4][4], B_inter[4][4], C_out[4][4], A_inter[4][5], B_inter[4+1][4], C_out[4][5], cws_inter[4][4], cws_inter[4][5]);
	compute_pe<4, 5>(A_inter[4][5], B_inter[4][5], C_out[4][5], A_inter[4][6], B_inter[4+1][5], C_out[4][6], cws_inter[4][5], cws_inter[4][6]);
	compute_pe<4, 6>(A_inter[4][6], B_inter[4][6], C_out[4][6], A_inter[4][7], B_inter[4+1][6], C_out[4][7], cws_inter[4][6], cws_inter[4][7]);
	compute_pe<4, 7>(A_inter[4][7], B_inter[4][7], C_out[4][7], A_inter[4][8], B_inter[4+1][7], C_out[4][8], cws_inter[4][7], cws_inter[4][8]);
	compute_pe<4, 8>(A_inter[4][8], B_inter[4][8], C_out[4][8], A_inter[4][9], B_inter[4+1][8],   Cout_s[4], cws_inter[4][8], cws_inter[4][9]);

	compute_pe<5, 0>(     Ain_s[5], B_inter[5][0], C_out[5][0], A_inter[5][1], B_inter[5+1][0], C_out[5][1],        cws_s[5], cws_inter[5][1]);
	compute_pe<5, 1>(A_inter[5][1], B_inter[5][1], C_out[5][1], A_inter[5][2], B_inter[5+1][1], C_out[5][2], cws_inter[5][1], cws_inter[5][2]);
	compute_pe<5, 2>(A_inter[5][2], B_inter[5][2], C_out[5][2], A_inter[5][3], B_inter[5+1][2], C_out[5][3], cws_inter[5][2], cws_inter[5][3]);
	compute_pe<5, 3>(A_inter[5][3], B_inter[5][3], C_out[5][3], A_inter[5][4], B_inter[5+1][3], C_out[5][4], cws_inter[5][3], cws_inter[5][4]);
	compute_pe<5, 4>(A_inter[5][4], B_inter[5][4], C_out[5][4], A_inter[5][5], B_inter[5+1][4], C_out[5][5], cws_inter[5][4], cws_inter[5][5]);
	compute_pe<5, 5>(A_inter[5][5], B_inter[5][5], C_out[5][5], A_inter[5][6], B_inter[5+1][5], C_out[5][6], cws_inter[5][5], cws_inter[5][6]);
	compute_pe<5, 6>(A_inter[5][6], B_inter[5][6], C_out[5][6], A_inter[5][7], B_inter[5+1][6], C_out[5][7], cws_inter[5][6], cws_inter[5][7]);
	compute_pe<5, 7>(A_inter[5][7], B_inter[5][7], C_out[5][7], A_inter[5][8], B_inter[5+1][7], C_out[5][8], cws_inter[5][7], cws_inter[5][8]);
	compute_pe<5, 8>(A_inter[5][8], B_inter[5][8], C_out[5][8], A_inter[5][9], B_inter[5+1][8],   Cout_s[5], cws_inter[5][8], cws_inter[5][9]);

	compute_pe<6, 0>(     Ain_s[6], B_inter[6][0], C_out[6][0], A_inter[6][1], B_inter[6+1][0], C_out[6][1],        cws_s[6], cws_inter[6][1]);
	compute_pe<6, 1>(A_inter[6][1], B_inter[6][1], C_out[6][1], A_inter[6][2], B_inter[6+1][1], C_out[6][2], cws_inter[6][1], cws_inter[6][2]);
	compute_pe<6, 2>(A_inter[6][2], B_inter[6][2], C_out[6][2], A_inter[6][3], B_inter[6+1][2], C_out[6][3], cws_inter[6][2], cws_inter[6][3]);
	compute_pe<6, 3>(A_inter[6][3], B_inter[6][3], C_out[6][3], A_inter[6][4], B_inter[6+1][3], C_out[6][4], cws_inter[6][3], cws_inter[6][4]);
	compute_pe<6, 4>(A_inter[6][4], B_inter[6][4], C_out[6][4], A_inter[6][5], B_inter[6+1][4], C_out[6][5], cws_inter[6][4], cws_inter[6][5]);
	compute_pe<6, 5>(A_inter[6][5], B_inter[6][5], C_out[6][5], A_inter[6][6], B_inter[6+1][5], C_out[6][6], cws_inter[6][5], cws_inter[6][6]);
	compute_pe<6, 6>(A_inter[6][6], B_inter[6][6], C_out[6][6], A_inter[6][7], B_inter[6+1][6], C_out[6][7], cws_inter[6][6], cws_inter[6][7]);
	compute_pe<6, 7>(A_inter[6][7], B_inter[6][7], C_out[6][7], A_inter[6][8], B_inter[6+1][7], C_out[6][8], cws_inter[6][7], cws_inter[6][8]);
	compute_pe<6, 8>(A_inter[6][8], B_inter[6][8], C_out[6][8], A_inter[6][9], B_inter[6+1][8],   Cout_s[6], cws_inter[6][8], cws_inter[6][9]);

//	Loop_M:for(int i=0; i<TILE_M; i++)
//	{
//#pragma HLS UNROLL
//		compute_pe<0, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<0, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<0, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<0, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<0, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<0, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<0, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<0, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<0, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//	}

//	Loop_M:for(int i=0; i<TILE_M; i++)
//	{
//#pragma HLS UNROLL
//		Loop_N:for(int j=0; j<TILE_N; j++)
//		{
//#pragma HLS UNROLL
//			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);
//		}
//	}

	DRAIN_AB:{
		cwrap_s cws;
		cws_inter[0][9] >> cws;

		for(int i=1; i<TILE_M; i++)
		{
#pragma HLS UNROLL
			cwrap_s cws_t;
			cws_inter[i][9] >> cws_t;
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
//DO_PRAGMA(#pragma HLS STREAM variable=Ain_s depth=TILE_K)
	hls::stream<int> Bin_s[TILE_N];
#pragma HLS STREAM variable=Bin_s depth=DEPTH_TILE_K
//DO_PRAGMA(#pragma HLS STREAM variable=Bin_s depth=TILE_K)
	hls::stream<int> Cout_s[TILE_M];
#pragma HLS STREAM variable=Cout_s depth=DEPTH_TILE_N
//DO_PRAGMA(#pragma HLS STREAM variable=Cout_s depth=TILE_N)
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
//////////////////////////////////////////v14 end////////////////////////////////////////

////////////////////////////////////////////v13 start////////////////////////////////////////
//
//typedef struct {
//	int K_MIN;
//	bool write_back;
//	bool init;
//} cwrap_s;
//
//template<int MIN_R,int MID_C>
//void compute_pe(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, cwrap_s cws)
//{
//	int K = cws.K_MIN;
//	bool write_back = cws.write_back;
//	bool init = cws.init;
//
//#pragma HLS INLINE off
//	static int C_local;
//	int A_tmp, B_tmp;
//
//	if(init){
//		C_local = 0;
//	}
//
//	for(int k=0; k<K; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//		A_out << A_tmp;
//		B_out << B_tmp;
//	}
//
//	if(write_back){
//		C_out.write(C_local);
//		//tranfer neighbor PE's output
//		for(int k=0; k<MID_C; k++){
//#pragma HLS PIPELINE II=1
//			int tmp_in, tmp_out;
//			C_in  >> tmp_in;
//			tmp_out = tmp_in;
//			C_out << tmp_out;
//		}
//	}
//
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		in >> drain;
//	}
//}
//
//void Load_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N],
//		hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		hls::stream<cwrap_s> &cws0, bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX){
//
//	static int K_cnt;
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	for(int k=0; k<K_MIN; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i][k];
//			Ain_s[i] << tmp;
//		}
//
//		for(int j=0; j<TILE_N; j++){
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			Bin_s[j] << tmp;
//		}
//	}
//
//	bool wb = (K_cnt == K_MAX);
//
//	cwrap_s cws;
//	cws.K_MIN = K_MIN;
//	cws.write_back = wb;
//	cws.init = init;
//	cws0.write(cws);
//}
//
//void Compute_wrapper(hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		hls::stream<int> Cout_s[TILE_M], hls::stream<cwrap_s> &cws0, hls::stream<bool> &wb1){
//
//	hls::stream<int> A_inter[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_K
//	hls::stream<int> B_inter[TILE_M+1][TILE_N];
//#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_K
//	hls::stream<int> C_out[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_TILE_N
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_TILE_N
//
//	cwrap_s cws;
//	cws = cws0.read();
//
//	compute_pe<0, 0>(     Ain_s[0], Bin_s[0], C_out[0][0], A_inter[0][1], B_inter[0+1][0], C_out[0][1], cws);
//	compute_pe<0, 1>(A_inter[0][1], Bin_s[1], C_out[0][1], A_inter[0][2], B_inter[0+1][1], C_out[0][2], cws);
//	compute_pe<0, 2>(A_inter[0][2], Bin_s[2], C_out[0][2], A_inter[0][3], B_inter[0+1][2], C_out[0][3], cws);
//	compute_pe<0, 3>(A_inter[0][3], Bin_s[3], C_out[0][3], A_inter[0][4], B_inter[0+1][3], C_out[0][4], cws);
//	compute_pe<0, 4>(A_inter[0][4], Bin_s[4], C_out[0][4], A_inter[0][5], B_inter[0+1][4], C_out[0][5], cws);
//	compute_pe<0, 5>(A_inter[0][5], Bin_s[5], C_out[0][5], A_inter[0][6], B_inter[0+1][5], C_out[0][6], cws);
//	compute_pe<0, 6>(A_inter[0][6], Bin_s[6], C_out[0][6], A_inter[0][7], B_inter[0+1][6], C_out[0][7], cws);
//	compute_pe<0, 7>(A_inter[0][7], Bin_s[7], C_out[0][7], A_inter[0][8], B_inter[0+1][7], C_out[0][8], cws);
//	compute_pe<0, 8>(A_inter[0][8], Bin_s[8], C_out[0][8], A_inter[0][9], B_inter[0+1][8],   Cout_s[0], cws);
//
//	compute_pe<1, 0>(     Ain_s[1], B_inter[1][0], C_out[1][0], A_inter[1][1], B_inter[1+1][0], C_out[1][1], cws);
//	compute_pe<1, 1>(A_inter[1][1], B_inter[1][1], C_out[1][1], A_inter[1][2], B_inter[1+1][1], C_out[1][2], cws);
//	compute_pe<1, 2>(A_inter[1][2], B_inter[1][2], C_out[1][2], A_inter[1][3], B_inter[1+1][2], C_out[1][3], cws);
//	compute_pe<1, 3>(A_inter[1][3], B_inter[1][3], C_out[1][3], A_inter[1][4], B_inter[1+1][3], C_out[1][4], cws);
//	compute_pe<1, 4>(A_inter[1][4], B_inter[1][4], C_out[1][4], A_inter[1][5], B_inter[1+1][4], C_out[1][5], cws);
//	compute_pe<1, 5>(A_inter[1][5], B_inter[1][5], C_out[1][5], A_inter[1][6], B_inter[1+1][5], C_out[1][6], cws);
//	compute_pe<1, 6>(A_inter[1][6], B_inter[1][6], C_out[1][6], A_inter[1][7], B_inter[1+1][6], C_out[1][7], cws);
//	compute_pe<1, 7>(A_inter[1][7], B_inter[1][7], C_out[1][7], A_inter[1][8], B_inter[1+1][7], C_out[1][8], cws);
//	compute_pe<1, 8>(A_inter[1][8], B_inter[1][8], C_out[1][8], A_inter[1][9], B_inter[1+1][8],   Cout_s[1], cws);
//
//	compute_pe<2, 0>(     Ain_s[2], B_inter[2][0], C_out[2][0], A_inter[2][1], B_inter[2+1][0], C_out[2][1], cws);
//	compute_pe<2, 1>(A_inter[2][1], B_inter[2][1], C_out[2][1], A_inter[2][2], B_inter[2+1][1], C_out[2][2], cws);
//	compute_pe<2, 2>(A_inter[2][2], B_inter[2][2], C_out[2][2], A_inter[2][3], B_inter[2+1][2], C_out[2][3], cws);
//	compute_pe<2, 3>(A_inter[2][3], B_inter[2][3], C_out[2][3], A_inter[2][4], B_inter[2+1][3], C_out[2][4], cws);
//	compute_pe<2, 4>(A_inter[2][4], B_inter[2][4], C_out[2][4], A_inter[2][5], B_inter[2+1][4], C_out[2][5], cws);
//	compute_pe<2, 5>(A_inter[2][5], B_inter[2][5], C_out[2][5], A_inter[2][6], B_inter[2+1][5], C_out[2][6], cws);
//	compute_pe<2, 6>(A_inter[2][6], B_inter[2][6], C_out[2][6], A_inter[2][7], B_inter[2+1][6], C_out[2][7], cws);
//	compute_pe<2, 7>(A_inter[2][7], B_inter[2][7], C_out[2][7], A_inter[2][8], B_inter[2+1][7], C_out[2][8], cws);
//	compute_pe<2, 8>(A_inter[2][8], B_inter[2][8], C_out[2][8], A_inter[2][9], B_inter[2+1][8],   Cout_s[2], cws);
//
//	compute_pe<3, 0>(     Ain_s[3], B_inter[3][0], C_out[3][0], A_inter[3][1], B_inter[3+1][0], C_out[3][1], cws);
//	compute_pe<3, 1>(A_inter[3][1], B_inter[3][1], C_out[3][1], A_inter[3][2], B_inter[3+1][1], C_out[3][2], cws);
//	compute_pe<3, 2>(A_inter[3][2], B_inter[3][2], C_out[3][2], A_inter[3][3], B_inter[3+1][2], C_out[3][3], cws);
//	compute_pe<3, 3>(A_inter[3][3], B_inter[3][3], C_out[3][3], A_inter[3][4], B_inter[3+1][3], C_out[3][4], cws);
//	compute_pe<3, 4>(A_inter[3][4], B_inter[3][4], C_out[3][4], A_inter[3][5], B_inter[3+1][4], C_out[3][5], cws);
//	compute_pe<3, 5>(A_inter[3][5], B_inter[3][5], C_out[3][5], A_inter[3][6], B_inter[3+1][5], C_out[3][6], cws);
//	compute_pe<3, 6>(A_inter[3][6], B_inter[3][6], C_out[3][6], A_inter[3][7], B_inter[3+1][6], C_out[3][7], cws);
//	compute_pe<3, 7>(A_inter[3][7], B_inter[3][7], C_out[3][7], A_inter[3][8], B_inter[3+1][7], C_out[3][8], cws);
//	compute_pe<3, 8>(A_inter[3][8], B_inter[3][8], C_out[3][8], A_inter[3][9], B_inter[3+1][8],   Cout_s[3], cws);
//
//	compute_pe<4, 0>(     Ain_s[4], B_inter[4][0], C_out[4][0], A_inter[4][1], B_inter[4+1][0], C_out[4][1], cws);
//	compute_pe<4, 1>(A_inter[4][1], B_inter[4][1], C_out[4][1], A_inter[4][2], B_inter[4+1][1], C_out[4][2], cws);
//	compute_pe<4, 2>(A_inter[4][2], B_inter[4][2], C_out[4][2], A_inter[4][3], B_inter[4+1][2], C_out[4][3], cws);
//	compute_pe<4, 3>(A_inter[4][3], B_inter[4][3], C_out[4][3], A_inter[4][4], B_inter[4+1][3], C_out[4][4], cws);
//	compute_pe<4, 4>(A_inter[4][4], B_inter[4][4], C_out[4][4], A_inter[4][5], B_inter[4+1][4], C_out[4][5], cws);
//	compute_pe<4, 5>(A_inter[4][5], B_inter[4][5], C_out[4][5], A_inter[4][6], B_inter[4+1][5], C_out[4][6], cws);
//	compute_pe<4, 6>(A_inter[4][6], B_inter[4][6], C_out[4][6], A_inter[4][7], B_inter[4+1][6], C_out[4][7], cws);
//	compute_pe<4, 7>(A_inter[4][7], B_inter[4][7], C_out[4][7], A_inter[4][8], B_inter[4+1][7], C_out[4][8], cws);
//	compute_pe<4, 8>(A_inter[4][8], B_inter[4][8], C_out[4][8], A_inter[4][9], B_inter[4+1][8],   Cout_s[4], cws);
//
//	compute_pe<5, 0>(     Ain_s[5], B_inter[5][0], C_out[5][0], A_inter[5][1], B_inter[5+1][0], C_out[5][1], cws);
//	compute_pe<5, 1>(A_inter[5][1], B_inter[5][1], C_out[5][1], A_inter[5][2], B_inter[5+1][1], C_out[5][2], cws);
//	compute_pe<5, 2>(A_inter[5][2], B_inter[5][2], C_out[5][2], A_inter[5][3], B_inter[5+1][2], C_out[5][3], cws);
//	compute_pe<5, 3>(A_inter[5][3], B_inter[5][3], C_out[5][3], A_inter[5][4], B_inter[5+1][3], C_out[5][4], cws);
//	compute_pe<5, 4>(A_inter[5][4], B_inter[5][4], C_out[5][4], A_inter[5][5], B_inter[5+1][4], C_out[5][5], cws);
//	compute_pe<5, 5>(A_inter[5][5], B_inter[5][5], C_out[5][5], A_inter[5][6], B_inter[5+1][5], C_out[5][6], cws);
//	compute_pe<5, 6>(A_inter[5][6], B_inter[5][6], C_out[5][6], A_inter[5][7], B_inter[5+1][6], C_out[5][7], cws);
//	compute_pe<5, 7>(A_inter[5][7], B_inter[5][7], C_out[5][7], A_inter[5][8], B_inter[5+1][7], C_out[5][8], cws);
//	compute_pe<5, 8>(A_inter[5][8], B_inter[5][8], C_out[5][8], A_inter[5][9], B_inter[5+1][8],   Cout_s[5], cws);
//
//	compute_pe<6, 0>(     Ain_s[6], B_inter[6][0], C_out[6][0], A_inter[6][1], B_inter[6+1][0], C_out[6][1], cws);
//	compute_pe<6, 1>(A_inter[6][1], B_inter[6][1], C_out[6][1], A_inter[6][2], B_inter[6+1][1], C_out[6][2], cws);
//	compute_pe<6, 2>(A_inter[6][2], B_inter[6][2], C_out[6][2], A_inter[6][3], B_inter[6+1][2], C_out[6][3], cws);
//	compute_pe<6, 3>(A_inter[6][3], B_inter[6][3], C_out[6][3], A_inter[6][4], B_inter[6+1][3], C_out[6][4], cws);
//	compute_pe<6, 4>(A_inter[6][4], B_inter[6][4], C_out[6][4], A_inter[6][5], B_inter[6+1][4], C_out[6][5], cws);
//	compute_pe<6, 5>(A_inter[6][5], B_inter[6][5], C_out[6][5], A_inter[6][6], B_inter[6+1][5], C_out[6][6], cws);
//	compute_pe<6, 6>(A_inter[6][6], B_inter[6][6], C_out[6][6], A_inter[6][7], B_inter[6+1][6], C_out[6][7], cws);
//	compute_pe<6, 7>(A_inter[6][7], B_inter[6][7], C_out[6][7], A_inter[6][8], B_inter[6+1][7], C_out[6][8], cws);
//	compute_pe<6, 8>(A_inter[6][8], B_inter[6][8], C_out[6][8], A_inter[6][9], B_inter[6+1][8],   Cout_s[6], cws);
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		compute_pe<0, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
////		compute_pe<0, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
////		compute_pe<0, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
////		compute_pe<0, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
////		compute_pe<0, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
////		compute_pe<0, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
////		compute_pe<0, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
////		compute_pe<0, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
////		compute_pe<0, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
////	}
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		Loop_N:for(int j=0; j<TILE_N; j++)
////		{
////#pragma HLS UNROLL
////			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);
////		}
////	}
//
//	DRAIN_AB:{
//		for(int i=0; i<TILE_M; i++)
//		{
//#pragma HLS UNROLL
//			Drain(A_inter[i][TILE_N], cws.K_MIN);
//		}
//
//		for(int j=0; j<TILE_N; j++)
//		{
//#pragma HLS UNROLL
//			Drain(B_inter[TILE_M][j], cws.K_MIN);
//		}
//	}
//
//	wb1.write(cws.write_back);
//}
//
//void Write_wrapper(int C_local[TILE_M][TILE_N], hls::stream<int> Cout_s[TILE_M], int M_MIN, int N_MIN, int K_MIN, hls::stream<bool> &wb1)
//{
//	bool write_back = wb1.read();
//	if(write_back){
//		for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//			for(int i=0; i<TILE_M; i++)
//			{
//				int tmp_out;
//				Cout_s[i] >> tmp_out;
//				C_local[i][TILE_N-1-j] = tmp_out;
//			}
//		}
//	}
//}
//
//void Compute_SA(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int C_local[TILE_M][TILE_N], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
//{
//
//#pragma HLS DATAFLOW
//
//	hls::stream<int> Ain_s[TILE_M];
//#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Ain_s depth=TILE_K)
//	hls::stream<int> Bin_s[TILE_N];
//#pragma HLS STREAM variable=Bin_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Bin_s depth=TILE_K)
//	hls::stream<int> Cout_s[TILE_M];
//#pragma HLS STREAM variable=Cout_s depth=DEPTH_TILE_N
////DO_PRAGMA(#pragma HLS STREAM variable=Cout_s depth=TILE_N)
//
//	hls::stream<cwrap_s> cws0;
//	hls::stream<bool> wb1;
//
//	Load_wrapper( A_local, B_local, Ain_s, Bin_s, cws0, init, M_MIN, N_MIN, K_MIN, K_MAX);
//
//	Compute_wrapper( Ain_s, Bin_s, Cout_s, cws0, wb1);
//
//	Write_wrapper( C_local, Cout_s, M_MIN, N_MIN, K_MIN, wb1);
//
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS PIPELINE II=1
//				B_local[i][k] = B[base_offset + i*N_len + k];
//			}
//}
//
//void Load_AB_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int *A, int *B,
//		int K, int N, int i, int j, bool k_init, int M_MIN, int N_MIN, int K_MIN0[1], int k0[1])
//{
//
//	static int k;
//	if(k_init){
//		k = 0;
//	}else{
//		k += TILE_K;
//	}
//
//	int K_MIN = MIN(TILE_K, K-k);
//
//	Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//	Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//	K_MIN0[0] = K_MIN;
//	k0[0] = k;
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
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void DATAFLOW_Load_Compute(int C_local[TILE_M][TILE_N], int *A, int *B, int M, int N, int K, int kloops, int i, int j, int M_MIN, int N_MIN)
//{
//	static int A_local[TILE_M][TILE_K];
//#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
//	static int B_local[TILE_K][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2
//
//	int K_MIN0[1], k0[1];
//
//	Loop_K:for(int k=0; k<kloops; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
//#pragma HLS DATAFLOW
//		Load_AB_wrapper( A_local, B_local, A, B, K, N, i, j, k==0, M_MIN, N_MIN, K_MIN0, k0);
//
//		Compute_SA(A_local, B_local, C_local, k0[0]==0, M_MIN, N_MIN, K_MIN0[0], K);
//	}
//}
//
//void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops)//A[MxK]*B[KxN]=C[MxN]
//{
//#pragma HLS INTERFACE m_axi depth=384 port=A offset=slave bundle=DB_A
//#pragma HLS INTERFACE m_axi depth=768 port=B offset=slave bundle=DB_B
//#pragma HLS INTERFACE m_axi depth=2048 port=C offset=slave bundle=DB_C
//
//#pragma HLS INTERFACE s_axilite register port=return bundle=CB
//#pragma HLS INTERFACE s_axilite register port=M bundle=CB
//#pragma HLS INTERFACE s_axilite register port=N bundle=CB
//#pragma HLS INTERFACE s_axilite register port=K bundle=CB
//#pragma HLS INTERFACE s_axilite register port=kloops bundle=CB
//
//#pragma HLS INTERFACE s_axilite register port=A bundle=CB
//#pragma HLS INTERFACE s_axilite register port=B bundle=CB
//#pragma HLS INTERFACE s_axilite register port=C bundle=CB
//
//	static int C_local[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
//			N_MIN = MIN(TILE_N, N-j);
//
//			DATAFLOW_Load_Compute( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v13 end////////////////////////////////////////

////////////////////////////////////////////v12 start////////////////////////////////////////
//template<int MIN_R,int MID_C>
//void compute_pe(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, bool write_back, bool init)
//{
//#pragma HLS INLINE off
//	static int C_local;
//	int A_tmp, B_tmp;
//
//	if(init){
//		C_local = 0;
//	}
//
//	for(int k=0; k<K; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//		A_out << A_tmp;
//		B_out << B_tmp;
//	}
//
//	if(write_back){
//		C_out.write(C_local);
//	}
//
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		in >> drain;
//	}
//}
//
//void Load_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N],
//		hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		int *K_cnt0, bool *init0, bool init, int M_MIN, int N_MIN, int K_MIN){
//
//	static int K_cnt;
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	for(int k=0; k<K_MIN; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i][k];
//			Ain_s[i] << tmp;
//		}
//
//		for(int j=0; j<TILE_N; j++){
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			Bin_s[j] << tmp;
//		}
//	}
//
//	*K_cnt0 = K_cnt;
//	*init0 = init;
//}
//
//void Compute_wrapper(hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		hls::stream<int> Cout_s[TILE_M], bool init, int K_MIN, int K_MAX, int K_cnt, int *K_cnt1){
//
//	hls::stream<int> A_inter[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_K
//	hls::stream<int> B_inter[TILE_M+1][TILE_N];
//#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_K
//hls::stream<int> C_out[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_TILE_N
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_TILE_N
//
//	bool write_back = (K_cnt == K_MAX);
//
////#pragma HLS DATAFLOW
//	LOAD_AB:for(int k=0; k<K_MIN; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = Ain_s[i].read();
//			A_inter[i][0].write(tmp);
//		}
//
//		for(int j=0; j<TILE_N; j++){
//			int tmp = Bin_s[j].read();
//			B_inter[0][j].write(tmp);
//		}
//	}
//
//	COMPUTE_C:{
//		compute_pe<0, 0>(A_inter[0][0], B_inter[0][0], A_inter[0][1], B_inter[0+1][0], C_out[0][0], K_MIN, write_back, init);
//		compute_pe<0, 1>(A_inter[0][1], B_inter[0][1], A_inter[0][2], B_inter[0+1][1], C_out[0][1], K_MIN, write_back, init);
//		compute_pe<0, 2>(A_inter[0][2], B_inter[0][2], A_inter[0][3], B_inter[0+1][2], C_out[0][2], K_MIN, write_back, init);
//		compute_pe<0, 3>(A_inter[0][3], B_inter[0][3], A_inter[0][4], B_inter[0+1][3], C_out[0][3], K_MIN, write_back, init);
//		compute_pe<0, 4>(A_inter[0][4], B_inter[0][4], A_inter[0][5], B_inter[0+1][4], C_out[0][4], K_MIN, write_back, init);
//		compute_pe<0, 5>(A_inter[0][5], B_inter[0][5], A_inter[0][6], B_inter[0+1][5], C_out[0][5], K_MIN, write_back, init);
//		compute_pe<0, 6>(A_inter[0][6], B_inter[0][6], A_inter[0][7], B_inter[0+1][6], C_out[0][6], K_MIN, write_back, init);
//		compute_pe<0, 7>(A_inter[0][7], B_inter[0][7], A_inter[0][8], B_inter[0+1][7], C_out[0][7], K_MIN, write_back, init);
//		compute_pe<0, 8>(A_inter[0][8], B_inter[0][8], A_inter[0][9], B_inter[0+1][8], C_out[0][8], K_MIN, write_back, init);
//
//		compute_pe<1, 0>(A_inter[1][0], B_inter[1][0], A_inter[1][1], B_inter[1+1][0], C_out[1][0], K_MIN, write_back, init);
//		compute_pe<1, 1>(A_inter[1][1], B_inter[1][1], A_inter[1][2], B_inter[1+1][1], C_out[1][1], K_MIN, write_back, init);
//		compute_pe<1, 2>(A_inter[1][2], B_inter[1][2], A_inter[1][3], B_inter[1+1][2], C_out[1][2], K_MIN, write_back, init);
//		compute_pe<1, 3>(A_inter[1][3], B_inter[1][3], A_inter[1][4], B_inter[1+1][3], C_out[1][3], K_MIN, write_back, init);
//		compute_pe<1, 4>(A_inter[1][4], B_inter[1][4], A_inter[1][5], B_inter[1+1][4], C_out[1][4], K_MIN, write_back, init);
//		compute_pe<1, 5>(A_inter[1][5], B_inter[1][5], A_inter[1][6], B_inter[1+1][5], C_out[1][5], K_MIN, write_back, init);
//		compute_pe<1, 6>(A_inter[1][6], B_inter[1][6], A_inter[1][7], B_inter[1+1][6], C_out[1][6], K_MIN, write_back, init);
//		compute_pe<1, 7>(A_inter[1][7], B_inter[1][7], A_inter[1][8], B_inter[1+1][7], C_out[1][7], K_MIN, write_back, init);
//		compute_pe<1, 8>(A_inter[1][8], B_inter[1][8], A_inter[1][9], B_inter[1+1][8], C_out[1][8], K_MIN, write_back, init);
//
//		compute_pe<2, 0>(A_inter[2][0], B_inter[2][0], A_inter[2][1], B_inter[2+1][0], C_out[2][0], K_MIN, write_back, init);
//		compute_pe<2, 1>(A_inter[2][1], B_inter[2][1], A_inter[2][2], B_inter[2+1][1], C_out[2][1], K_MIN, write_back, init);
//		compute_pe<2, 2>(A_inter[2][2], B_inter[2][2], A_inter[2][3], B_inter[2+1][2], C_out[2][2], K_MIN, write_back, init);
//		compute_pe<2, 3>(A_inter[2][3], B_inter[2][3], A_inter[2][4], B_inter[2+1][3], C_out[2][3], K_MIN, write_back, init);
//		compute_pe<2, 4>(A_inter[2][4], B_inter[2][4], A_inter[2][5], B_inter[2+1][4], C_out[2][4], K_MIN, write_back, init);
//		compute_pe<2, 5>(A_inter[2][5], B_inter[2][5], A_inter[2][6], B_inter[2+1][5], C_out[2][5], K_MIN, write_back, init);
//		compute_pe<2, 6>(A_inter[2][6], B_inter[2][6], A_inter[2][7], B_inter[2+1][6], C_out[2][6], K_MIN, write_back, init);
//		compute_pe<2, 7>(A_inter[2][7], B_inter[2][7], A_inter[2][8], B_inter[2+1][7], C_out[2][7], K_MIN, write_back, init);
//		compute_pe<2, 8>(A_inter[2][8], B_inter[2][8], A_inter[2][9], B_inter[2+1][8], C_out[2][8], K_MIN, write_back, init);
//
//		compute_pe<3, 0>(A_inter[3][0], B_inter[3][0], A_inter[3][1], B_inter[3+1][0], C_out[3][0], K_MIN, write_back, init);
//		compute_pe<3, 1>(A_inter[3][1], B_inter[3][1], A_inter[3][2], B_inter[3+1][1], C_out[3][1], K_MIN, write_back, init);
//		compute_pe<3, 2>(A_inter[3][2], B_inter[3][2], A_inter[3][3], B_inter[3+1][2], C_out[3][2], K_MIN, write_back, init);
//		compute_pe<3, 3>(A_inter[3][3], B_inter[3][3], A_inter[3][4], B_inter[3+1][3], C_out[3][3], K_MIN, write_back, init);
//		compute_pe<3, 4>(A_inter[3][4], B_inter[3][4], A_inter[3][5], B_inter[3+1][4], C_out[3][4], K_MIN, write_back, init);
//		compute_pe<3, 5>(A_inter[3][5], B_inter[3][5], A_inter[3][6], B_inter[3+1][5], C_out[3][5], K_MIN, write_back, init);
//		compute_pe<3, 6>(A_inter[3][6], B_inter[3][6], A_inter[3][7], B_inter[3+1][6], C_out[3][6], K_MIN, write_back, init);
//		compute_pe<3, 7>(A_inter[3][7], B_inter[3][7], A_inter[3][8], B_inter[3+1][7], C_out[3][7], K_MIN, write_back, init);
//		compute_pe<3, 8>(A_inter[3][8], B_inter[3][8], A_inter[3][9], B_inter[3+1][8], C_out[3][8], K_MIN, write_back, init);
//
//		compute_pe<4, 0>(A_inter[4][0], B_inter[4][0], A_inter[4][1], B_inter[4+1][0], C_out[4][0], K_MIN, write_back, init);
//		compute_pe<4, 1>(A_inter[4][1], B_inter[4][1], A_inter[4][2], B_inter[4+1][1], C_out[4][1], K_MIN, write_back, init);
//		compute_pe<4, 2>(A_inter[4][2], B_inter[4][2], A_inter[4][3], B_inter[4+1][2], C_out[4][2], K_MIN, write_back, init);
//		compute_pe<4, 3>(A_inter[4][3], B_inter[4][3], A_inter[4][4], B_inter[4+1][3], C_out[4][3], K_MIN, write_back, init);
//		compute_pe<4, 4>(A_inter[4][4], B_inter[4][4], A_inter[4][5], B_inter[4+1][4], C_out[4][4], K_MIN, write_back, init);
//		compute_pe<4, 5>(A_inter[4][5], B_inter[4][5], A_inter[4][6], B_inter[4+1][5], C_out[4][5], K_MIN, write_back, init);
//		compute_pe<4, 6>(A_inter[4][6], B_inter[4][6], A_inter[4][7], B_inter[4+1][6], C_out[4][6], K_MIN, write_back, init);
//		compute_pe<4, 7>(A_inter[4][7], B_inter[4][7], A_inter[4][8], B_inter[4+1][7], C_out[4][7], K_MIN, write_back, init);
//		compute_pe<4, 8>(A_inter[4][8], B_inter[4][8], A_inter[4][9], B_inter[4+1][8], C_out[4][8], K_MIN, write_back, init);
//
//		compute_pe<5, 0>(A_inter[5][0], B_inter[5][0], A_inter[5][1], B_inter[5+1][0], C_out[5][0], K_MIN, write_back, init);
//		compute_pe<5, 1>(A_inter[5][1], B_inter[5][1], A_inter[5][2], B_inter[5+1][1], C_out[5][1], K_MIN, write_back, init);
//		compute_pe<5, 2>(A_inter[5][2], B_inter[5][2], A_inter[5][3], B_inter[5+1][2], C_out[5][2], K_MIN, write_back, init);
//		compute_pe<5, 3>(A_inter[5][3], B_inter[5][3], A_inter[5][4], B_inter[5+1][3], C_out[5][3], K_MIN, write_back, init);
//		compute_pe<5, 4>(A_inter[5][4], B_inter[5][4], A_inter[5][5], B_inter[5+1][4], C_out[5][4], K_MIN, write_back, init);
//		compute_pe<5, 5>(A_inter[5][5], B_inter[5][5], A_inter[5][6], B_inter[5+1][5], C_out[5][5], K_MIN, write_back, init);
//		compute_pe<5, 6>(A_inter[5][6], B_inter[5][6], A_inter[5][7], B_inter[5+1][6], C_out[5][6], K_MIN, write_back, init);
//		compute_pe<5, 7>(A_inter[5][7], B_inter[5][7], A_inter[5][8], B_inter[5+1][7], C_out[5][7], K_MIN, write_back, init);
//		compute_pe<5, 8>(A_inter[5][8], B_inter[5][8], A_inter[5][9], B_inter[5+1][8], C_out[5][8], K_MIN, write_back, init);
//
//		compute_pe<6, 0>(A_inter[6][0], B_inter[6][0], A_inter[6][1], B_inter[6+1][0], C_out[6][0], K_MIN, write_back, init);
//		compute_pe<6, 1>(A_inter[6][1], B_inter[6][1], A_inter[6][2], B_inter[6+1][1], C_out[6][1], K_MIN, write_back, init);
//		compute_pe<6, 2>(A_inter[6][2], B_inter[6][2], A_inter[6][3], B_inter[6+1][2], C_out[6][2], K_MIN, write_back, init);
//		compute_pe<6, 3>(A_inter[6][3], B_inter[6][3], A_inter[6][4], B_inter[6+1][3], C_out[6][3], K_MIN, write_back, init);
//		compute_pe<6, 4>(A_inter[6][4], B_inter[6][4], A_inter[6][5], B_inter[6+1][4], C_out[6][4], K_MIN, write_back, init);
//		compute_pe<6, 5>(A_inter[6][5], B_inter[6][5], A_inter[6][6], B_inter[6+1][5], C_out[6][5], K_MIN, write_back, init);
//		compute_pe<6, 6>(A_inter[6][6], B_inter[6][6], A_inter[6][7], B_inter[6+1][6], C_out[6][6], K_MIN, write_back, init);
//		compute_pe<6, 7>(A_inter[6][7], B_inter[6][7], A_inter[6][8], B_inter[6+1][7], C_out[6][7], K_MIN, write_back, init);
//		compute_pe<6, 8>(A_inter[6][8], B_inter[6][8], A_inter[6][9], B_inter[6+1][8], C_out[6][8], K_MIN, write_back, init);
//	}
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		compute_pe<0, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
////		compute_pe<0, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
////		compute_pe<0, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
////		compute_pe<0, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
////		compute_pe<0, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
////		compute_pe<0, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
////		compute_pe<0, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
////		compute_pe<0, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
////		compute_pe<0, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
////	}
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		Loop_N:for(int j=0; j<TILE_N; j++)
////		{
////#pragma HLS UNROLL
////			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);
////		}
////	}
//
//	DRAIN_AB:{
//		for(int i=0; i<TILE_M; i++)
//		{
//#pragma HLS UNROLL
//			Drain(A_inter[i][TILE_N], K_MIN);
//		}
//
//		for(int j=0; j<TILE_N; j++)
//		{
//#pragma HLS UNROLL
//			Drain(B_inter[TILE_M][j], K_MIN);
//		}
//	}
//
//	if(write_back){
////tranfer neighbor PE's output
//		for(int j=TILE_N-1; j>-1; j--){
//			for(int k=0; k<TILE_N-1-j; k++){
//#pragma HLS PIPELINE II=1
//				for(int i=0; i<TILE_M; i++){
//					int tmp_in, tmp_out;
//					C_out[i][j+1] >> tmp_in;
//					tmp_out = tmp_in;
//					C_out[i][j]   << tmp_out;
//				}
//			}
//		}
//
//		for(int i=0; i<TILE_M; i++)
//		{
//#pragma HLS UNROLL
//			for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//				int tmp_in, tmp_out;
//				C_out[i][0] >> tmp_in;
//				tmp_out = tmp_in;
//				Cout_s[i] << tmp_out;
//			}
//		}
//	}
//
//	*K_cnt1 = K_cnt;
//}
//
//void Write_wrapper(int C_local[TILE_M][TILE_N], hls::stream<int> Cout_s[TILE_M],
//		int M_MIN, int N_MIN, int K_MIN, int K_cnt, int K_MAX){
//	if(K_cnt == K_MAX){
//		for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//			for(int i=0; i<TILE_M; i++)
//			{
//				int tmp_out;
//				Cout_s[i] >> tmp_out;
//				if((i < M_MIN) && (j < N_MIN))
//					C_local[i][j] = tmp_out;
//			}
//		}
//	}
//}
//
//void Compute_SA(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int C_local[TILE_M][TILE_N], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
//{
//
//#pragma HLS DATAFLOW
//
//	hls::stream<int> Ain_s[TILE_M];
//#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Ain_s depth=TILE_K)
//	hls::stream<int> Bin_s[TILE_N];
//#pragma HLS STREAM variable=Bin_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Bin_s depth=TILE_K)
//hls::stream<int> Cout_s[TILE_M];
//#pragma HLS STREAM variable=Cout_s depth=DEPTH_TILE_N
////DO_PRAGMA(#pragma HLS STREAM variable=Cout_s depth=TILE_N)
//
//	int K_cnt0[1], K_cnt1[1];
//	bool init0[1];
//
//	Load_wrapper( A_local, B_local, Ain_s, Bin_s, K_cnt0, init0, init, M_MIN, N_MIN, K_MIN);
//
//	Compute_wrapper( Ain_s, Bin_s, Cout_s, init0[0], K_MIN, K_MAX, K_cnt0[0], K_cnt1);
//
//	Write_wrapper( C_local, Cout_s, M_MIN, N_MIN, K_MIN, K_cnt1[0], K_MAX);
//
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS PIPELINE II=1
//				B_local[i][k] = B[base_offset + i*N_len + k];
//			}
//}
//
//void Load_AB_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int *A, int *B,
//		int K, int N, int i, int j, bool k_init, int M_MIN, int N_MIN, int K_MIN0[1], int k0[1])
//{
//
//	static int k;
//	if(k_init){
//		k = 0;
//	}else{
//		k += TILE_K;
//	}
//
//	int K_MIN = MIN(TILE_K, K-k);
//
//	Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//	Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//	K_MIN0[0] = K_MIN;
//	k0[0] = k;
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
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void DATAFLOW_Load_Compute(int C_local[TILE_M][TILE_N], int *A, int *B, int M, int N, int K, int kloops, int i, int j, int M_MIN, int N_MIN)
//{
//	static int A_local[TILE_M][TILE_K];
//#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
//	static int B_local[TILE_K][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2
//
//	int K_MIN0[1], k0[1];
//
//	Loop_K:for(int k=0; k<kloops; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
//#pragma HLS DATAFLOW
//		Load_AB_wrapper( A_local, B_local, A, B, K, N, i, j, k==0, M_MIN, N_MIN, K_MIN0, k0);
//
//		Compute_SA(A_local, B_local, C_local, k0[0]==0, M_MIN, N_MIN, K_MIN0[0], K);
//	}
//}
//
//void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops)//A[MxK]*B[KxN]=C[MxN]
//{
//#pragma HLS INTERFACE m_axi depth=384 port=A offset=slave bundle=DB_A
//#pragma HLS INTERFACE m_axi depth=768 port=B offset=slave bundle=DB_B
//#pragma HLS INTERFACE m_axi depth=2048 port=C offset=slave bundle=DB_C
//
//#pragma HLS INTERFACE s_axilite register port=return bundle=CB
//#pragma HLS INTERFACE s_axilite register port=M bundle=CB
//#pragma HLS INTERFACE s_axilite register port=N bundle=CB
//#pragma HLS INTERFACE s_axilite register port=K bundle=CB
//#pragma HLS INTERFACE s_axilite register port=kloops bundle=CB
//
//#pragma HLS INTERFACE s_axilite register port=A bundle=CB
//#pragma HLS INTERFACE s_axilite register port=B bundle=CB
//#pragma HLS INTERFACE s_axilite register port=C bundle=CB
//
//	static int C_local[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
//			N_MIN = MIN(TILE_N, N-j);
//
//			DATAFLOW_Load_Compute( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v12 end////////////////////////////////////////

////////////////////////////////////////////v11 start////////////////////////////////////////
//template<int MIN_R,int MID_C>
//void compute_pe(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init,
//		int *C_local_array, int *k_cnt_array)
//{
//#pragma HLS INLINE off
//	int C_local;
//	int k_cnt;
//
//	int A_tmp, B_tmp;
//	if(init){
//		C_local = 0;
//		k_cnt = 0;
//	}else
//	{
//		C_local = *C_local_array;
//		k_cnt = *k_cnt_array;
//	}
//
//	for(int k=0; k<K; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//		A_out << A_tmp;
//		B_out << B_tmp;
//		k_cnt++;
//	}
//
//	if(k_cnt == K_MAX){
//		C_out.write(C_local);
//	}else{
//		*C_local_array = C_local;
//		*k_cnt_array = k_cnt;
//	}
//
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		in >> drain;
//	}
//}
//
//void Load_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N],
//		hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		int *K_cnt0, bool *init0, bool init, int M_MIN, int N_MIN, int K_MIN){
//
//	static int K_cnt;
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	for(int k=0; k<K_MIN; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i][k];
//			Ain_s[i] << tmp;
//		}
//
//		for(int j=0; j<TILE_N; j++){
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			Bin_s[j] << tmp;
//		}
//	}
//
//	*K_cnt0 = K_cnt;
//	*init0 = init;
//}
//
//void Compute_wrapper(hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		hls::stream<int> Cout_s[TILE_M], bool init, int K_MIN, int K_MAX, int K_cnt, int *K_cnt1){
//
//	hls::stream<int> A_inter[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_K
//	hls::stream<int> B_inter[TILE_M+1][TILE_N];
//#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_K
//hls::stream<int> C_out[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_TILE_N
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_TILE_N
//
//	static int C_local_array[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=C_local_array complete dim=0
//	static int k_cnt_array[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=k_cnt_array complete dim=0
//
//	LOAD_AB:for(int k=0; k<K_MIN; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = Ain_s[i].read();
//			A_inter[i][0].write(tmp);
//		}
//
//		for(int j=0; j<TILE_N; j++){
//			int tmp = Bin_s[j].read();
//			B_inter[0][j].write(tmp);
//		}
//	}
//
//	COMPUTE_C:{
//		int i;
//		i = 0;
//		compute_pe<0, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<0, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<0, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<0, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<0, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<0, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<0, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<0, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<0, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 1;
//		compute_pe<1, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<1, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<1, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<1, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<1, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<1, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<1, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<1, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<1, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 2;
//		compute_pe<2, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<2, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<2, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<2, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<2, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<2, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<2, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<2, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<2, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 3;
//		compute_pe<3, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<3, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<3, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<3, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<3, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<3, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<3, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<3, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<3, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 4;
//		compute_pe<4, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<4, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<4, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<4, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<4, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<4, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<4, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<4, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<4, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 5;
//		compute_pe<5, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<5, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<5, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<5, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<5, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<5, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<5, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<5, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<5, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 6;
//		compute_pe<6, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<6, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<6, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<6, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<6, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<6, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<6, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<6, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<6, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//	}
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		compute_pe<0, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
////		compute_pe<0, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
////		compute_pe<0, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
////		compute_pe<0, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
////		compute_pe<0, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
////		compute_pe<0, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
////		compute_pe<0, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
////		compute_pe<0, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
////		compute_pe<0, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
////	}
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		Loop_N:for(int j=0; j<TILE_N; j++)
////		{
////#pragma HLS UNROLL
////			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);
////		}
////	}
//
//	DRAIN_AB:{
//		for(int i=0; i<TILE_M; i++)
//		{
//#pragma HLS UNROLL
//			Drain(A_inter[i][TILE_N], K_MIN);
//		}
//
//		for(int j=0; j<TILE_N; j++)
//		{
//#pragma HLS UNROLL
//			Drain(B_inter[TILE_M][j], K_MIN);
//		}
//	}
//
//	if(K_cnt == K_MAX){
////tranfer neighbor PE's output
//		for(int j=TILE_N-1; j>-1; j--){
//			for(int k=0; k<TILE_N-1-j; k++){
//#pragma HLS PIPELINE II=1
//				for(int i=0; i<TILE_M; i++){
//					int tmp_in, tmp_out;
//					C_out[i][j+1] >> tmp_in;
//					tmp_out = tmp_in;
//					C_out[i][j]   << tmp_out;
//				}
//			}
//		}
//
//		for(int i=0; i<TILE_M; i++)
//		{
//#pragma HLS UNROLL
//			for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//				int tmp_in, tmp_out;
//				C_out[i][0] >> tmp_in;
//				tmp_out = tmp_in;
//				Cout_s[i] << tmp_out;
//			}
//		}
//	}
//
//	*K_cnt1 = K_cnt;
//}
//
//void Write_wrapper(int C_local[TILE_M][TILE_N], hls::stream<int> Cout_s[TILE_M],
//		int M_MIN, int N_MIN, int K_MIN, int K_cnt, int K_MAX){
//	if(K_cnt == K_MAX){
//		for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//			for(int i=0; i<TILE_M; i++)
//			{
//				int tmp_out;
//				Cout_s[i] >> tmp_out;
//				if((i < M_MIN) && (j < N_MIN))
//					C_local[i][j] = tmp_out;
//			}
//		}
//	}
//}
//
//void Compute_SA(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int C_local[TILE_M][TILE_N], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
//{
//
//#pragma HLS DATAFLOW
//
//	hls::stream<int> Ain_s[TILE_M];
//#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Ain_s depth=TILE_K)
//	hls::stream<int> Bin_s[TILE_N];
//#pragma HLS STREAM variable=Bin_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Bin_s depth=TILE_K)
//hls::stream<int> Cout_s[TILE_M];
//#pragma HLS STREAM variable=Cout_s depth=DEPTH_TILE_N
////DO_PRAGMA(#pragma HLS STREAM variable=Cout_s depth=TILE_N)
//
//	int K_cnt0[1], K_cnt1[1];
//	bool init0[1];
//
//	Load_wrapper( A_local, B_local, Ain_s, Bin_s, K_cnt0, init0, init, M_MIN, N_MIN, K_MIN);
//
//	Compute_wrapper( Ain_s, Bin_s, Cout_s, init0[0], K_MIN, K_MAX, K_cnt0[0], K_cnt1);
//
//	Write_wrapper( C_local, Cout_s, M_MIN, N_MIN, K_MIN, K_cnt1[0], K_MAX);
//
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS PIPELINE II=1
//				B_local[i][k] = B[base_offset + i*N_len + k];
//			}
//}
//
//void Load_AB_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int *A, int *B,
//		int K, int N, int i, int j, bool k_init, int M_MIN, int N_MIN, int K_MIN0[1], int k0[1])
//{
//
//	static int k;
//	if(k_init){
//		k = 0;
//	}else{
//		k += TILE_K;
//	}
//
//	int K_MIN = MIN(TILE_K, K-k);
//
//	Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//	Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//	K_MIN0[0] = K_MIN;
//	k0[0] = k;
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
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void DATAFLOW_Load_Compute(int C_local[TILE_M][TILE_N], int *A, int *B, int M, int N, int K, int kloops, int i, int j, int M_MIN, int N_MIN)
//{
//	static int A_local[TILE_M][TILE_K];
//#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
//	static int B_local[TILE_K][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2
//
//	int K_MIN0[1], k0[1];
//	Loop_K:for(int k=0; k<kloops; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
//#pragma HLS DATAFLOW
//		Load_AB_wrapper( A_local, B_local, A, B, K, N, i, j, k==0, M_MIN, N_MIN, K_MIN0, k0);
//
//		Compute_SA(A_local, B_local, C_local, k0[0]==0, M_MIN, N_MIN, K_MIN0[0], K);
//	}
//}
//
//void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops)//A[MxK]*B[KxN]=C[MxN]
//{
//#pragma HLS INTERFACE m_axi depth=384 port=A offset=slave bundle=DB_A
//#pragma HLS INTERFACE m_axi depth=768 port=B offset=slave bundle=DB_B
//#pragma HLS INTERFACE m_axi depth=2048 port=C offset=slave bundle=DB_C
//
//#pragma HLS INTERFACE s_axilite register port=return bundle=CB
//#pragma HLS INTERFACE s_axilite register port=M bundle=CB
//#pragma HLS INTERFACE s_axilite register port=N bundle=CB
//#pragma HLS INTERFACE s_axilite register port=K bundle=CB
//#pragma HLS INTERFACE s_axilite register port=kloops bundle=CB
//
//#pragma HLS INTERFACE s_axilite register port=A bundle=CB
//#pragma HLS INTERFACE s_axilite register port=B bundle=CB
//#pragma HLS INTERFACE s_axilite register port=C bundle=CB
//
//	static int C_local[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
//			N_MIN = MIN(TILE_N, N-j);
//
//			DATAFLOW_Load_Compute( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v11 end////////////////////////////////////////

////////////////////////////////////////////v10 start////////////////////////////////////////
//#define MAX(x,y) ((x)>(y)?(x):(y))
//#define MIN(x,y) ((x)<(y)?(x):(y))
//
//#define TILE_M 7
//#define TILE_N 9
//#define TILE_K 5
//
//const int DEPTH_TILE_M = TILE_M;
//const int DEPTH_TILE_N = TILE_N;
//const int DEPTH_TILE_K = TILE_K;
//
//template<int MIN_R,int MID_C>
//void compute_pe(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init,
//		int *C_local_array, int *k_cnt_array)
//{
//#pragma HLS INLINE off
//	int C_local;
//	int k_cnt;
//
//	int A_tmp, B_tmp;
//	if(init){
//		C_local = 0;
//		k_cnt = 0;
//	}else
//	{
//		C_local = *C_local_array;
//		k_cnt = *k_cnt_array;
//	}
//
//	for(int k=0; k<K; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//		A_out << A_tmp;
//		B_out << B_tmp;
//		k_cnt++;
//	}
//
//	if(k_cnt == K_MAX){
//		C_out.write(C_local);
//	}else{
//		*C_local_array = C_local;
//		*k_cnt_array = k_cnt;
//	}
//
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		in >> drain;
//	}
//}
//
//void Load_wrapper(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N],
//		hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		int *K_cnt0, bool *init0, bool init, int M_MIN, int N_MIN, int K_MIN){
//
//	static int K_cnt;
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	for(int k=0; k<K_MIN; k++)
//	{
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i][k];
//			Ain_s[i] << tmp;
//		}
//
//		for(int j=0; j<TILE_N; j++){
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			Bin_s[j] << tmp;
//		}
//	}
//
//	*K_cnt0 = K_cnt;
//	*init0 = init;
//}
//
//void Compute_wrapper(hls::stream<int> Ain_s[TILE_M], hls::stream<int> Bin_s[TILE_N],
//		hls::stream<int> Cout_s[TILE_M], bool init, int K_MIN, int K_MAX, int K_cnt, int *K_cnt1){
//
//	hls::stream<int> A_inter[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=A_inter dim=1 depth=TILE_K)
////DO_PRAGMA(#pragma HLS STREAM variable=A_inter dim=2 depth=TILE_K)
//	hls::stream<int> B_inter[TILE_M+1][TILE_N];
//#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_K
//#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=B_inter dim=1 depth=TILE_K)
////DO_PRAGMA(#pragma HLS STREAM variable=B_inter dim=2 depth=TILE_K)
//hls::stream<int> C_out[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_TILE_N
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_TILE_N
////DO_PRAGMA(#pragma HLS STREAM variable=C_out dim=1 depth=TILE_N)
////DO_PRAGMA(#pragma HLS STREAM variable=C_out dim=2 depth=TILE_N)
//
//	static int C_local_array[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=C_local_array complete dim=0
//	static int k_cnt_array[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=k_cnt_array complete dim=0
//
//	LOAD_AB:for(int k=0; k<K_MIN; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_K)
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = Ain_s[i].read();
//			A_inter[i][0].write(tmp);
//		}
//
//		for(int j=0; j<TILE_N; j++){
//			int tmp = Bin_s[j].read();
//			B_inter[0][j].write(tmp);
//		}
//	}
//
//	COMPUTE_C:{
//		int i;
//		i = 0;
//		compute_pe<0, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<0, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<0, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<0, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<0, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<0, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<0, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<0, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<0, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 1;
//		compute_pe<1, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<1, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<1, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<1, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<1, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<1, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<1, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<1, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<1, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 2;
//		compute_pe<2, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<2, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<2, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<2, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<2, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<2, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<2, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<2, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<2, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 3;
//		compute_pe<3, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<3, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<3, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<3, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<3, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<3, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<3, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<3, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<3, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 4;
//		compute_pe<4, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<4, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<4, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<4, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<4, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<4, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<4, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<4, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<4, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 5;
//		compute_pe<5, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<5, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<5, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<5, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<5, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<5, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<5, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<5, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<5, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//		i = 6;
//		compute_pe<6, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
//		compute_pe<6, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
//		compute_pe<6, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
//		compute_pe<6, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
//		compute_pe<6, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
//		compute_pe<6, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
//		compute_pe<6, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
//		compute_pe<6, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
//		compute_pe<6, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
//	}
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		compute_pe<0, 0>(A_inter[i][0], B_inter[i][0], A_inter[i][1], B_inter[i+1][0], C_out[i][0], K_MIN, K_MAX, init, &C_local_array[i][0], &k_cnt_array[i][0]);
////		compute_pe<0, 1>(A_inter[i][1], B_inter[i][1], A_inter[i][2], B_inter[i+1][1], C_out[i][1], K_MIN, K_MAX, init, &C_local_array[i][1], &k_cnt_array[i][1]);
////		compute_pe<0, 2>(A_inter[i][2], B_inter[i][2], A_inter[i][3], B_inter[i+1][2], C_out[i][2], K_MIN, K_MAX, init, &C_local_array[i][2], &k_cnt_array[i][2]);
////		compute_pe<0, 3>(A_inter[i][3], B_inter[i][3], A_inter[i][4], B_inter[i+1][3], C_out[i][3], K_MIN, K_MAX, init, &C_local_array[i][3], &k_cnt_array[i][3]);
////		compute_pe<0, 4>(A_inter[i][4], B_inter[i][4], A_inter[i][5], B_inter[i+1][4], C_out[i][4], K_MIN, K_MAX, init, &C_local_array[i][4], &k_cnt_array[i][4]);
////		compute_pe<0, 5>(A_inter[i][5], B_inter[i][5], A_inter[i][6], B_inter[i+1][5], C_out[i][5], K_MIN, K_MAX, init, &C_local_array[i][5], &k_cnt_array[i][5]);
////		compute_pe<0, 6>(A_inter[i][6], B_inter[i][6], A_inter[i][7], B_inter[i+1][6], C_out[i][6], K_MIN, K_MAX, init, &C_local_array[i][6], &k_cnt_array[i][6]);
////		compute_pe<0, 7>(A_inter[i][7], B_inter[i][7], A_inter[i][8], B_inter[i+1][7], C_out[i][7], K_MIN, K_MAX, init, &C_local_array[i][7], &k_cnt_array[i][7]);
////		compute_pe<0, 8>(A_inter[i][8], B_inter[i][8], A_inter[i][9], B_inter[i+1][8], C_out[i][8], K_MIN, K_MAX, init, &C_local_array[i][8], &k_cnt_array[i][8]);
////	}
//
////	Loop_M:for(int i=0; i<TILE_M; i++)
////	{
////#pragma HLS UNROLL
////		Loop_N:for(int j=0; j<TILE_N; j++)
////		{
////#pragma HLS UNROLL
////			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);
////		}
////	}
//	DRAIN_AB:{
//		for(int i=0; i<TILE_M; i++)
//		{
//#pragma HLS UNROLL
//			Drain(A_inter[i][TILE_N], K_MIN);
//		}
//
//		for(int j=0; j<TILE_N; j++)
//		{
//#pragma HLS UNROLL
//			Drain(B_inter[TILE_M][j], K_MIN);
//		}
//	}
//
//	if(K_cnt == K_MAX){
////tranfer neighbor PE's output
//		for(int j=TILE_N-1; j>-1; j--){
//			for(int k=0; k<TILE_N-1-j; k++){
//#pragma HLS PIPELINE II=1
//				for(int i=0; i<TILE_M; i++){
//					int tmp_in, tmp_out;
//					C_out[i][j+1] >> tmp_in;
//					tmp_out = tmp_in;
//					C_out[i][j]   << tmp_out;
//				}
//			}
//		}
//
//		for(int i=0; i<TILE_M; i++)
//		{
//#pragma HLS UNROLL
//			for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//				int tmp_in, tmp_out;
//				C_out[i][0] >> tmp_in;
//				tmp_out = tmp_in;
//				Cout_s[i] << tmp_out;
//			}
//		}
//	}
//
//	*K_cnt1 = K_cnt;
//}
//
//void Write_wrapper(int C_local[TILE_M][TILE_N], hls::stream<int> Cout_s[TILE_M],
//		int M_MIN, int N_MIN, int K_MIN, int K_cnt, int K_MAX){
//	if(K_cnt == K_MAX){
//		for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//			for(int i=0; i<TILE_M; i++)
//			{
//				int tmp_out;
//				Cout_s[i] >> tmp_out;
//				if((i < M_MIN) && (j < N_MIN))
//					C_local[i][j] = tmp_out;
//			}
//		}
//	}
//}
//
//void Compute_SA(int A_local[TILE_M][TILE_K], int B_local[TILE_K][TILE_N], int C_local[TILE_M][TILE_N], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
//{
//
//#pragma HLS DATAFLOW
//
//	hls::stream<int> Ain_s[TILE_M];
//#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Ain_s depth=TILE_K)
//	hls::stream<int> Bin_s[TILE_N];
//#pragma HLS STREAM variable=Bin_s depth=DEPTH_TILE_K
////DO_PRAGMA(#pragma HLS STREAM variable=Bin_s depth=TILE_K)
//hls::stream<int> Cout_s[TILE_M];
//#pragma HLS STREAM variable=Cout_s depth=DEPTH_TILE_N
////DO_PRAGMA(#pragma HLS STREAM variable=Cout_s depth=TILE_N)
//
//	int K_cnt0[1], K_cnt1[1];
//	bool init0[1];
//
//	Load_wrapper( A_local, B_local, Ain_s, Bin_s, K_cnt0, init0, init, M_MIN, N_MIN, K_MIN);
//
//	Compute_wrapper( Ain_s, Bin_s, Cout_s, init0[0], K_MIN, K_MAX, K_cnt0[0], K_cnt1);
//
//	Write_wrapper( C_local, Cout_s, M_MIN, N_MIN, K_MIN, K_cnt1[0], K_MAX);
//
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
//	static int B_local[TILE_K][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2
//	static int C_local[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN, K_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= TILE_N)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
//			N_MIN = MIN(TILE_N, N-j);
//			Loop_K:for(int k=0; k<K; k+= TILE_K)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
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
////////////////////////////////////////////v10 end////////////////////////////////////////

////////////////////////////////////////////v9 start////////////////////////////////////////
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
//	int k_cnt;
//public:
//	void compute(hls::stream<int> &A_in, hls::stream<int> &B_in,
//			hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init);
//};
//
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &B_in,
//		hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init)
//{
//	int A_tmp, B_tmp;
//	if(init){
//		C_local = 0;
//		k_cnt = 0;
//	}
//
//	for(int k=0; k<K; k++)
//	{
//#pragma HLS PIPELINE II=1
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//
//		A_out << A_tmp;
//		B_out << B_tmp;
//		k_cnt++;
//	}
//
//	if(k_cnt == K_MAX){
//		C_out << C_local;
//	}
//
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//#pragma HLS PIPELINE II=1
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
//	hls::stream<int> C_out[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=C_out
//
//	static PE_cls PE_array[TILE_M][TILE_N];
//	static int K_cnt;
//
//#pragma HLS DATAFLOW
//
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	for(int k=0; k<K_MIN; k++)
//	{
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<TILE_M; i++){
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i][k];
//			A_inter[i][0] << tmp;
//		}
//	}
//
//	for(int k=0; k<K_MIN; k++)
//	{
//#pragma HLS PIPELINE II=1
//		for(int j=0; j<TILE_N; j++){
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			B_inter[0][j] << tmp;
//		}
//	}
//
//	Loop_M:for(int i=0; i<TILE_M; i++){
//#pragma HLS UNROLL
//		Loop_N:for(int j=0; j<TILE_N; j++)
//		{
//#pragma HLS UNROLL
//			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);
//		}
//	}
//
//	for(int i=0; i<TILE_M; i++)
//	{
//#pragma HLS UNROLL
//		Drain(A_inter[i][TILE_N], K_MIN);
//	}
//
//	for(int j=0; j<TILE_N; j++)
//	{
//#pragma HLS UNROLL
//		Drain(B_inter[TILE_M][j], K_MIN);
//	}
//
//	if(K_cnt == K_MAX){
////tranfer neighbor PE's output
//		for(int j=TILE_N-1; j>=0; j--)
//			for(int k=0; k<TILE_N-1-j; k++){
//#pragma HLS PIPELINE II=1
//				for(int i=0; i<TILE_M; i++){
//					int tmp_in, tmp_out;
//					C_out[i][j+1] >> tmp_in;
//					tmp_out = tmp_in;
//					C_out[i][j]   << tmp_out;
//				}
//			}
//
//		for(int j=0; j<TILE_N; j++){
//#pragma HLS PIPELINE II=1
//			for(int i=0; i<TILE_M; i++)
//			{
//				int tmp_out;
//				C_out[i][0] >> tmp_out;
//				if((i < M_MIN) && (j < N_MIN))
//					C_local[i][j] = tmp_out;
//			}
//		}
//	}
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS PIPELINE II=1
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
//#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
//	static int B_local[TILE_K][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2
//	static int C_local[TILE_M][TILE_N];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
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
////////////////////////////////////////////v9 end////////////////////////////////////////

////////////////////////////////////////////v8 start////////////////////////////////////////
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
//	int k_cnt;
//public:
////	int trans_num;
//	void compute(hls::stream<int> &A_in, hls::stream<int> &B_in, //hls::stream<int> &C_in,
//			hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init);
//};
//
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &B_in, //hls::stream<int> &C_in,
//		hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init)
//{
//	int A_tmp, B_tmp;
//	if(init){
//		C_local = 0;
//		k_cnt = 0;
//	}
//
//	for(int k=0; k<K; k++)
//	{
//		A_in >> A_tmp;
//		B_in >> B_tmp;
//
//		C_local += A_tmp * B_tmp;
//
//		A_out << A_tmp;
//		B_out << B_tmp;
//		k_cnt++;
//	}
//
//	if(k_cnt == K_MAX){
//		C_out << C_local;
//	}
//
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
//	hls::stream<int> C_out[TILE_M][TILE_N+1];
//#pragma HLS STREAM variable=C_out
//
//	static PE_cls PE_array[TILE_M][TILE_N];
//	static int K_cnt;
//
//	for(int i=0; i<TILE_M; i++)
//		for(int k=0; k<K_MIN; k++)
//		{
//			int tmp = 0;
//			if(i<M_MIN)
//				tmp = A_local[i][k];
//			A_inter[i][0] << tmp;
//		}
//
//	for(int j=0; j<TILE_N; j++)
//		for(int k=0; k<K_MIN; k++)
//		{
//			int tmp = 0;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			B_inter[0][j] << tmp;
//		}
//
//	Loop_M:for(int i=0; i<TILE_M; i++)
//		Loop_N:for(int j=0; j<TILE_N; j++)
//		{
//			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], /*C_out[i][j+1],*/ A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);
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
//
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	if(K_cnt == K_MAX){
//		for(int i=0; i<TILE_M; i++)//tranfer neighbor PE's output
//			for(int j=TILE_N-1; j>=0; j--)
//			{
//				for(int k=0; k<TILE_N-1-j; k++){
//					int tmp_in, tmp_out;
//					C_out[i][j+1] >> tmp_in;
//					tmp_out = tmp_in;
//					C_out[i][j]   << tmp_out;
//				}
//			}
//
//		for(int j=0; j<TILE_N; j++)
//			for(int i=0; i<TILE_M; i++)
//			{
//#pragma HLS PIPELINE II=1
//				int tmp_out;
//				C_out[i][0] >> tmp_out;
////				C_out[i][j] >> tmp_out;
//				if((i < M_MIN) && (j < N_MIN))
//					C_local[i][j] = tmp_out;
//			}
//	}
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
////////////////////////////////////////////v8 end////////////////////////////////////////

////////////////////////////////////////////v7 start////////////////////////////////////////
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
//	int k_cnt;
//public:
////	int trans_num;
//	void compute(hls::stream<int> &A_in, hls::stream<int> &B_in, //hls::stream<int> &C_in,
//			hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init);
//};
//
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &B_in, //hls::stream<int> &C_in,
//		hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int K, int K_MAX, bool init)
//{
//	int A_tmp, B_tmp;
//	if(init){
//		C_local = 0;
//		k_cnt = 0;
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
//		k_cnt++;
//	}
//
//	if(k_cnt == K_MAX){
//		C_out << C_local;
////		for(int k=0; k<trans_num; k++)
////			C_out.write(C_in.read());
//	}
//
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
//	hls::stream<int> C_out[TILE_M][TILE_N+1];
////	hls::stream<int> C_out[TILE_M][TILE_N];
//#pragma HLS STREAM variable=C_out
//
//	static PE_cls PE_array[TILE_M][TILE_N];
//	static int K_cnt;
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
////			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], A_inter[i][j+1], B_inter[i+1][j], &C_local[i][j], K_MIN, K_MAX, init);//, C_out[i][j], K_MIN);
////			PE_array[i][j].trans_num = TILE_N-j-1;
//			PE_array[i][j].compute(A_inter[i][j], B_inter[i][j], /*C_out[i][j+1],*/ A_inter[i][j+1], B_inter[i+1][j], C_out[i][j], K_MIN, K_MAX, init);//, C_out[i][j], K_MIN);
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
//
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	if(K_cnt == K_MAX){
//		for(int i=0; i<TILE_M; i++)
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			for(int j=0; j<TILE_N; j++)
//			{
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_N)
//#pragma HLS PIPELINE II=1
//				int tmp_out;
////				C_out[i][0] >> tmp_out;
//				tmp_out = C_out[i][j].read();
////				C_out[i][j] >> tmp_out;
//				if((i < M_MIN) && (j < N_MIN))
//					C_local[i][j] = tmp_out;
//			}
//	}
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
////////////////////////////////////////////v7 end////////////////////////////////////////

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


