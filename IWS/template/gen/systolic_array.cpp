
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

PE_array[0][0].fillB<0,0>(Bin_s[0], B_inter[1][0]);
PE_array[0][1].fillB<0,1>(Bin_s[1], B_inter[1][1]);
PE_array[0][2].fillB<0,2>(Bin_s[2], B_inter[1][2]);
PE_array[0][3].fillB<0,3>(Bin_s[3], B_inter[1][3]);
PE_array[0][4].fillB<0,4>(Bin_s[4], B_inter[1][4]);
PE_array[0][5].fillB<0,5>(Bin_s[5], B_inter[1][5]);
PE_array[0][6].fillB<0,6>(Bin_s[6], B_inter[1][6]);
PE_array[0][7].fillB<0,7>(Bin_s[7], B_inter[1][7]);
PE_array[0][8].fillB<0,8>(Bin_s[8], B_inter[1][8]);
PE_array[0][9].fillB<0,9>(Bin_s[9], B_inter[1][9]);
PE_array[0][10].fillB<0,10>(Bin_s[10], B_inter[1][10]);

PE_array[1][0].fillB<1,0>(B_inter[1][0], B_inter[2][0]);
PE_array[1][1].fillB<1,1>(B_inter[1][1], B_inter[2][1]);
PE_array[1][2].fillB<1,2>(B_inter[1][2], B_inter[2][2]);
PE_array[1][3].fillB<1,3>(B_inter[1][3], B_inter[2][3]);
PE_array[1][4].fillB<1,4>(B_inter[1][4], B_inter[2][4]);
PE_array[1][5].fillB<1,5>(B_inter[1][5], B_inter[2][5]);
PE_array[1][6].fillB<1,6>(B_inter[1][6], B_inter[2][6]);
PE_array[1][7].fillB<1,7>(B_inter[1][7], B_inter[2][7]);
PE_array[1][8].fillB<1,8>(B_inter[1][8], B_inter[2][8]);
PE_array[1][9].fillB<1,9>(B_inter[1][9], B_inter[2][9]);
PE_array[1][10].fillB<1,10>(B_inter[1][10], B_inter[2][10]);

PE_array[2][0].fillB<2,0>(B_inter[2][0], B_inter[3][0]);
PE_array[2][1].fillB<2,1>(B_inter[2][1], B_inter[3][1]);
PE_array[2][2].fillB<2,2>(B_inter[2][2], B_inter[3][2]);
PE_array[2][3].fillB<2,3>(B_inter[2][3], B_inter[3][3]);
PE_array[2][4].fillB<2,4>(B_inter[2][4], B_inter[3][4]);
PE_array[2][5].fillB<2,5>(B_inter[2][5], B_inter[3][5]);
PE_array[2][6].fillB<2,6>(B_inter[2][6], B_inter[3][6]);
PE_array[2][7].fillB<2,7>(B_inter[2][7], B_inter[3][7]);
PE_array[2][8].fillB<2,8>(B_inter[2][8], B_inter[3][8]);
PE_array[2][9].fillB<2,9>(B_inter[2][9], B_inter[3][9]);
PE_array[2][10].fillB<2,10>(B_inter[2][10], B_inter[3][10]);

PE_array[3][0].fillB<3,0>(B_inter[3][0], B_inter[4][0]);
PE_array[3][1].fillB<3,1>(B_inter[3][1], B_inter[4][1]);
PE_array[3][2].fillB<3,2>(B_inter[3][2], B_inter[4][2]);
PE_array[3][3].fillB<3,3>(B_inter[3][3], B_inter[4][3]);
PE_array[3][4].fillB<3,4>(B_inter[3][4], B_inter[4][4]);
PE_array[3][5].fillB<3,5>(B_inter[3][5], B_inter[4][5]);
PE_array[3][6].fillB<3,6>(B_inter[3][6], B_inter[4][6]);
PE_array[3][7].fillB<3,7>(B_inter[3][7], B_inter[4][7]);
PE_array[3][8].fillB<3,8>(B_inter[3][8], B_inter[4][8]);
PE_array[3][9].fillB<3,9>(B_inter[3][9], B_inter[4][9]);
PE_array[3][10].fillB<3,10>(B_inter[3][10], B_inter[4][10]);

PE_array[4][0].fillB<4,0>(B_inter[4][0], B_inter[5][0]);
PE_array[4][1].fillB<4,1>(B_inter[4][1], B_inter[5][1]);
PE_array[4][2].fillB<4,2>(B_inter[4][2], B_inter[5][2]);
PE_array[4][3].fillB<4,3>(B_inter[4][3], B_inter[5][3]);
PE_array[4][4].fillB<4,4>(B_inter[4][4], B_inter[5][4]);
PE_array[4][5].fillB<4,5>(B_inter[4][5], B_inter[5][5]);
PE_array[4][6].fillB<4,6>(B_inter[4][6], B_inter[5][6]);
PE_array[4][7].fillB<4,7>(B_inter[4][7], B_inter[5][7]);
PE_array[4][8].fillB<4,8>(B_inter[4][8], B_inter[5][8]);
PE_array[4][9].fillB<4,9>(B_inter[4][9], B_inter[5][9]);
PE_array[4][10].fillB<4,10>(B_inter[4][10], B_inter[5][10]);

PE_array[5][0].fillB<5,0>(B_inter[5][0], B_inter[6][0]);
PE_array[5][1].fillB<5,1>(B_inter[5][1], B_inter[6][1]);
PE_array[5][2].fillB<5,2>(B_inter[5][2], B_inter[6][2]);
PE_array[5][3].fillB<5,3>(B_inter[5][3], B_inter[6][3]);
PE_array[5][4].fillB<5,4>(B_inter[5][4], B_inter[6][4]);
PE_array[5][5].fillB<5,5>(B_inter[5][5], B_inter[6][5]);
PE_array[5][6].fillB<5,6>(B_inter[5][6], B_inter[6][6]);
PE_array[5][7].fillB<5,7>(B_inter[5][7], B_inter[6][7]);
PE_array[5][8].fillB<5,8>(B_inter[5][8], B_inter[6][8]);
PE_array[5][9].fillB<5,9>(B_inter[5][9], B_inter[6][9]);
PE_array[5][10].fillB<5,10>(B_inter[5][10], B_inter[6][10]);

PE_array[6][0].fillB<6,0>(B_inter[6][0], B_inter[7][0]);
PE_array[6][1].fillB<6,1>(B_inter[6][1], B_inter[7][1]);
PE_array[6][2].fillB<6,2>(B_inter[6][2], B_inter[7][2]);
PE_array[6][3].fillB<6,3>(B_inter[6][3], B_inter[7][3]);
PE_array[6][4].fillB<6,4>(B_inter[6][4], B_inter[7][4]);
PE_array[6][5].fillB<6,5>(B_inter[6][5], B_inter[7][5]);
PE_array[6][6].fillB<6,6>(B_inter[6][6], B_inter[7][6]);
PE_array[6][7].fillB<6,7>(B_inter[6][7], B_inter[7][7]);
PE_array[6][8].fillB<6,8>(B_inter[6][8], B_inter[7][8]);
PE_array[6][9].fillB<6,9>(B_inter[6][9], B_inter[7][9]);
PE_array[6][10].fillB<6,10>(B_inter[6][10], B_inter[7][10]);

PE_array[7][0].fillB<7,0>(B_inter[7][0], B_inter[8][0]);
PE_array[7][1].fillB<7,1>(B_inter[7][1], B_inter[8][1]);
PE_array[7][2].fillB<7,2>(B_inter[7][2], B_inter[8][2]);
PE_array[7][3].fillB<7,3>(B_inter[7][3], B_inter[8][3]);
PE_array[7][4].fillB<7,4>(B_inter[7][4], B_inter[8][4]);
PE_array[7][5].fillB<7,5>(B_inter[7][5], B_inter[8][5]);
PE_array[7][6].fillB<7,6>(B_inter[7][6], B_inter[8][6]);
PE_array[7][7].fillB<7,7>(B_inter[7][7], B_inter[8][7]);
PE_array[7][8].fillB<7,8>(B_inter[7][8], B_inter[8][8]);
PE_array[7][9].fillB<7,9>(B_inter[7][9], B_inter[8][9]);
PE_array[7][10].fillB<7,10>(B_inter[7][10], B_inter[8][10]);

PE_array[8][0].fillB<8,0>(B_inter[8][0], B_inter[9][0]);
PE_array[8][1].fillB<8,1>(B_inter[8][1], B_inter[9][1]);
PE_array[8][2].fillB<8,2>(B_inter[8][2], B_inter[9][2]);
PE_array[8][3].fillB<8,3>(B_inter[8][3], B_inter[9][3]);
PE_array[8][4].fillB<8,4>(B_inter[8][4], B_inter[9][4]);
PE_array[8][5].fillB<8,5>(B_inter[8][5], B_inter[9][5]);
PE_array[8][6].fillB<8,6>(B_inter[8][6], B_inter[9][6]);
PE_array[8][7].fillB<8,7>(B_inter[8][7], B_inter[9][7]);
PE_array[8][8].fillB<8,8>(B_inter[8][8], B_inter[9][8]);
PE_array[8][9].fillB<8,9>(B_inter[8][9], B_inter[9][9]);
PE_array[8][10].fillB<8,10>(B_inter[8][10], B_inter[9][10]);

PE_array[9][0].fillB<9,0>(B_inter[9][0], B_inter[10][0]);
PE_array[9][1].fillB<9,1>(B_inter[9][1], B_inter[10][1]);
PE_array[9][2].fillB<9,2>(B_inter[9][2], B_inter[10][2]);
PE_array[9][3].fillB<9,3>(B_inter[9][3], B_inter[10][3]);
PE_array[9][4].fillB<9,4>(B_inter[9][4], B_inter[10][4]);
PE_array[9][5].fillB<9,5>(B_inter[9][5], B_inter[10][5]);
PE_array[9][6].fillB<9,6>(B_inter[9][6], B_inter[10][6]);
PE_array[9][7].fillB<9,7>(B_inter[9][7], B_inter[10][7]);
PE_array[9][8].fillB<9,8>(B_inter[9][8], B_inter[10][8]);
PE_array[9][9].fillB<9,9>(B_inter[9][9], B_inter[10][9]);
PE_array[9][10].fillB<9,10>(B_inter[9][10], B_inter[10][10]);

PE_array[10][0].fillB<10,0>(B_inter[10][0], B_inter[11][0]);
PE_array[10][1].fillB<10,1>(B_inter[10][1], B_inter[11][1]);
PE_array[10][2].fillB<10,2>(B_inter[10][2], B_inter[11][2]);
PE_array[10][3].fillB<10,3>(B_inter[10][3], B_inter[11][3]);
PE_array[10][4].fillB<10,4>(B_inter[10][4], B_inter[11][4]);
PE_array[10][5].fillB<10,5>(B_inter[10][5], B_inter[11][5]);
PE_array[10][6].fillB<10,6>(B_inter[10][6], B_inter[11][6]);
PE_array[10][7].fillB<10,7>(B_inter[10][7], B_inter[11][7]);
PE_array[10][8].fillB<10,8>(B_inter[10][8], B_inter[11][8]);
PE_array[10][9].fillB<10,9>(B_inter[10][9], B_inter[11][9]);
PE_array[10][10].fillB<10,10>(B_inter[10][10], B_inter[11][10]);

PE_array[11][0].fillB<11,0>(B_inter[11][0], B_inter[12][0]);
PE_array[11][1].fillB<11,1>(B_inter[11][1], B_inter[12][1]);
PE_array[11][2].fillB<11,2>(B_inter[11][2], B_inter[12][2]);
PE_array[11][3].fillB<11,3>(B_inter[11][3], B_inter[12][3]);
PE_array[11][4].fillB<11,4>(B_inter[11][4], B_inter[12][4]);
PE_array[11][5].fillB<11,5>(B_inter[11][5], B_inter[12][5]);
PE_array[11][6].fillB<11,6>(B_inter[11][6], B_inter[12][6]);
PE_array[11][7].fillB<11,7>(B_inter[11][7], B_inter[12][7]);
PE_array[11][8].fillB<11,8>(B_inter[11][8], B_inter[12][8]);
PE_array[11][9].fillB<11,9>(B_inter[11][9], B_inter[12][9]);
PE_array[11][10].fillB<11,10>(B_inter[11][10], B_inter[12][10]);

PE_array[12][0].fillB<12,0>(B_inter[12][0], B_inter[13][0]);
PE_array[12][1].fillB<12,1>(B_inter[12][1], B_inter[13][1]);
PE_array[12][2].fillB<12,2>(B_inter[12][2], B_inter[13][2]);
PE_array[12][3].fillB<12,3>(B_inter[12][3], B_inter[13][3]);
PE_array[12][4].fillB<12,4>(B_inter[12][4], B_inter[13][4]);
PE_array[12][5].fillB<12,5>(B_inter[12][5], B_inter[13][5]);
PE_array[12][6].fillB<12,6>(B_inter[12][6], B_inter[13][6]);
PE_array[12][7].fillB<12,7>(B_inter[12][7], B_inter[13][7]);
PE_array[12][8].fillB<12,8>(B_inter[12][8], B_inter[13][8]);
PE_array[12][9].fillB<12,9>(B_inter[12][9], B_inter[13][9]);
PE_array[12][10].fillB<12,10>(B_inter[12][10], B_inter[13][10]);



PE_array[0][0].compute<0,0>(     Ain_s[0],        M_MIN_s[0], C_out[0][0], A_inter[0][1], M_MIN_inter[0][1], C_out[1][0]);
PE_array[0][1].compute<0,1>(A_inter[0][1], M_MIN_inter[0][1], C_out[0][1], A_inter[0][2], M_MIN_inter[0][2], C_out[1][1]);
PE_array[0][2].compute<0,2>(A_inter[0][2], M_MIN_inter[0][2], C_out[0][2], A_inter[0][3], M_MIN_inter[0][3], C_out[1][2]);
PE_array[0][3].compute<0,3>(A_inter[0][3], M_MIN_inter[0][3], C_out[0][3], A_inter[0][4], M_MIN_inter[0][4], C_out[1][3]);
PE_array[0][4].compute<0,4>(A_inter[0][4], M_MIN_inter[0][4], C_out[0][4], A_inter[0][5], M_MIN_inter[0][5], C_out[1][4]);
PE_array[0][5].compute<0,5>(A_inter[0][5], M_MIN_inter[0][5], C_out[0][5], A_inter[0][6], M_MIN_inter[0][6], C_out[1][5]);
PE_array[0][6].compute<0,6>(A_inter[0][6], M_MIN_inter[0][6], C_out[0][6], A_inter[0][7], M_MIN_inter[0][7], C_out[1][6]);
PE_array[0][7].compute<0,7>(A_inter[0][7], M_MIN_inter[0][7], C_out[0][7], A_inter[0][8], M_MIN_inter[0][8], C_out[1][7]);
PE_array[0][8].compute<0,8>(A_inter[0][8], M_MIN_inter[0][8], C_out[0][8], A_inter[0][9], M_MIN_inter[0][9], C_out[1][8]);
PE_array[0][9].compute<0,9>(A_inter[0][9], M_MIN_inter[0][9], C_out[0][9], A_inter[0][10], M_MIN_inter[0][10], C_out[1][9]);
PE_array[0][10].compute<0,10>(A_inter[0][10], M_MIN_inter[0][10], C_out[0][10], A_inter[0][11], M_MIN_inter[0][11], C_out[1][10]);

PE_array[1][0].compute<1,0>(     Ain_s[1],        M_MIN_s[1], C_out[1][0], A_inter[1][1], M_MIN_inter[1][1], C_out[2][0]);
PE_array[1][1].compute<1,1>(A_inter[1][1], M_MIN_inter[1][1], C_out[1][1], A_inter[1][2], M_MIN_inter[1][2], C_out[2][1]);
PE_array[1][2].compute<1,2>(A_inter[1][2], M_MIN_inter[1][2], C_out[1][2], A_inter[1][3], M_MIN_inter[1][3], C_out[2][2]);
PE_array[1][3].compute<1,3>(A_inter[1][3], M_MIN_inter[1][3], C_out[1][3], A_inter[1][4], M_MIN_inter[1][4], C_out[2][3]);
PE_array[1][4].compute<1,4>(A_inter[1][4], M_MIN_inter[1][4], C_out[1][4], A_inter[1][5], M_MIN_inter[1][5], C_out[2][4]);
PE_array[1][5].compute<1,5>(A_inter[1][5], M_MIN_inter[1][5], C_out[1][5], A_inter[1][6], M_MIN_inter[1][6], C_out[2][5]);
PE_array[1][6].compute<1,6>(A_inter[1][6], M_MIN_inter[1][6], C_out[1][6], A_inter[1][7], M_MIN_inter[1][7], C_out[2][6]);
PE_array[1][7].compute<1,7>(A_inter[1][7], M_MIN_inter[1][7], C_out[1][7], A_inter[1][8], M_MIN_inter[1][8], C_out[2][7]);
PE_array[1][8].compute<1,8>(A_inter[1][8], M_MIN_inter[1][8], C_out[1][8], A_inter[1][9], M_MIN_inter[1][9], C_out[2][8]);
PE_array[1][9].compute<1,9>(A_inter[1][9], M_MIN_inter[1][9], C_out[1][9], A_inter[1][10], M_MIN_inter[1][10], C_out[2][9]);
PE_array[1][10].compute<1,10>(A_inter[1][10], M_MIN_inter[1][10], C_out[1][10], A_inter[1][11], M_MIN_inter[1][11], C_out[2][10]);

PE_array[2][0].compute<2,0>(     Ain_s[2],        M_MIN_s[2], C_out[2][0], A_inter[2][1], M_MIN_inter[2][1], C_out[3][0]);
PE_array[2][1].compute<2,1>(A_inter[2][1], M_MIN_inter[2][1], C_out[2][1], A_inter[2][2], M_MIN_inter[2][2], C_out[3][1]);
PE_array[2][2].compute<2,2>(A_inter[2][2], M_MIN_inter[2][2], C_out[2][2], A_inter[2][3], M_MIN_inter[2][3], C_out[3][2]);
PE_array[2][3].compute<2,3>(A_inter[2][3], M_MIN_inter[2][3], C_out[2][3], A_inter[2][4], M_MIN_inter[2][4], C_out[3][3]);
PE_array[2][4].compute<2,4>(A_inter[2][4], M_MIN_inter[2][4], C_out[2][4], A_inter[2][5], M_MIN_inter[2][5], C_out[3][4]);
PE_array[2][5].compute<2,5>(A_inter[2][5], M_MIN_inter[2][5], C_out[2][5], A_inter[2][6], M_MIN_inter[2][6], C_out[3][5]);
PE_array[2][6].compute<2,6>(A_inter[2][6], M_MIN_inter[2][6], C_out[2][6], A_inter[2][7], M_MIN_inter[2][7], C_out[3][6]);
PE_array[2][7].compute<2,7>(A_inter[2][7], M_MIN_inter[2][7], C_out[2][7], A_inter[2][8], M_MIN_inter[2][8], C_out[3][7]);
PE_array[2][8].compute<2,8>(A_inter[2][8], M_MIN_inter[2][8], C_out[2][8], A_inter[2][9], M_MIN_inter[2][9], C_out[3][8]);
PE_array[2][9].compute<2,9>(A_inter[2][9], M_MIN_inter[2][9], C_out[2][9], A_inter[2][10], M_MIN_inter[2][10], C_out[3][9]);
PE_array[2][10].compute<2,10>(A_inter[2][10], M_MIN_inter[2][10], C_out[2][10], A_inter[2][11], M_MIN_inter[2][11], C_out[3][10]);

PE_array[3][0].compute<3,0>(     Ain_s[3],        M_MIN_s[3], C_out[3][0], A_inter[3][1], M_MIN_inter[3][1], C_out[4][0]);
PE_array[3][1].compute<3,1>(A_inter[3][1], M_MIN_inter[3][1], C_out[3][1], A_inter[3][2], M_MIN_inter[3][2], C_out[4][1]);
PE_array[3][2].compute<3,2>(A_inter[3][2], M_MIN_inter[3][2], C_out[3][2], A_inter[3][3], M_MIN_inter[3][3], C_out[4][2]);
PE_array[3][3].compute<3,3>(A_inter[3][3], M_MIN_inter[3][3], C_out[3][3], A_inter[3][4], M_MIN_inter[3][4], C_out[4][3]);
PE_array[3][4].compute<3,4>(A_inter[3][4], M_MIN_inter[3][4], C_out[3][4], A_inter[3][5], M_MIN_inter[3][5], C_out[4][4]);
PE_array[3][5].compute<3,5>(A_inter[3][5], M_MIN_inter[3][5], C_out[3][5], A_inter[3][6], M_MIN_inter[3][6], C_out[4][5]);
PE_array[3][6].compute<3,6>(A_inter[3][6], M_MIN_inter[3][6], C_out[3][6], A_inter[3][7], M_MIN_inter[3][7], C_out[4][6]);
PE_array[3][7].compute<3,7>(A_inter[3][7], M_MIN_inter[3][7], C_out[3][7], A_inter[3][8], M_MIN_inter[3][8], C_out[4][7]);
PE_array[3][8].compute<3,8>(A_inter[3][8], M_MIN_inter[3][8], C_out[3][8], A_inter[3][9], M_MIN_inter[3][9], C_out[4][8]);
PE_array[3][9].compute<3,9>(A_inter[3][9], M_MIN_inter[3][9], C_out[3][9], A_inter[3][10], M_MIN_inter[3][10], C_out[4][9]);
PE_array[3][10].compute<3,10>(A_inter[3][10], M_MIN_inter[3][10], C_out[3][10], A_inter[3][11], M_MIN_inter[3][11], C_out[4][10]);

PE_array[4][0].compute<4,0>(     Ain_s[4],        M_MIN_s[4], C_out[4][0], A_inter[4][1], M_MIN_inter[4][1], C_out[5][0]);
PE_array[4][1].compute<4,1>(A_inter[4][1], M_MIN_inter[4][1], C_out[4][1], A_inter[4][2], M_MIN_inter[4][2], C_out[5][1]);
PE_array[4][2].compute<4,2>(A_inter[4][2], M_MIN_inter[4][2], C_out[4][2], A_inter[4][3], M_MIN_inter[4][3], C_out[5][2]);
PE_array[4][3].compute<4,3>(A_inter[4][3], M_MIN_inter[4][3], C_out[4][3], A_inter[4][4], M_MIN_inter[4][4], C_out[5][3]);
PE_array[4][4].compute<4,4>(A_inter[4][4], M_MIN_inter[4][4], C_out[4][4], A_inter[4][5], M_MIN_inter[4][5], C_out[5][4]);
PE_array[4][5].compute<4,5>(A_inter[4][5], M_MIN_inter[4][5], C_out[4][5], A_inter[4][6], M_MIN_inter[4][6], C_out[5][5]);
PE_array[4][6].compute<4,6>(A_inter[4][6], M_MIN_inter[4][6], C_out[4][6], A_inter[4][7], M_MIN_inter[4][7], C_out[5][6]);
PE_array[4][7].compute<4,7>(A_inter[4][7], M_MIN_inter[4][7], C_out[4][7], A_inter[4][8], M_MIN_inter[4][8], C_out[5][7]);
PE_array[4][8].compute<4,8>(A_inter[4][8], M_MIN_inter[4][8], C_out[4][8], A_inter[4][9], M_MIN_inter[4][9], C_out[5][8]);
PE_array[4][9].compute<4,9>(A_inter[4][9], M_MIN_inter[4][9], C_out[4][9], A_inter[4][10], M_MIN_inter[4][10], C_out[5][9]);
PE_array[4][10].compute<4,10>(A_inter[4][10], M_MIN_inter[4][10], C_out[4][10], A_inter[4][11], M_MIN_inter[4][11], C_out[5][10]);

PE_array[5][0].compute<5,0>(     Ain_s[5],        M_MIN_s[5], C_out[5][0], A_inter[5][1], M_MIN_inter[5][1], C_out[6][0]);
PE_array[5][1].compute<5,1>(A_inter[5][1], M_MIN_inter[5][1], C_out[5][1], A_inter[5][2], M_MIN_inter[5][2], C_out[6][1]);
PE_array[5][2].compute<5,2>(A_inter[5][2], M_MIN_inter[5][2], C_out[5][2], A_inter[5][3], M_MIN_inter[5][3], C_out[6][2]);
PE_array[5][3].compute<5,3>(A_inter[5][3], M_MIN_inter[5][3], C_out[5][3], A_inter[5][4], M_MIN_inter[5][4], C_out[6][3]);
PE_array[5][4].compute<5,4>(A_inter[5][4], M_MIN_inter[5][4], C_out[5][4], A_inter[5][5], M_MIN_inter[5][5], C_out[6][4]);
PE_array[5][5].compute<5,5>(A_inter[5][5], M_MIN_inter[5][5], C_out[5][5], A_inter[5][6], M_MIN_inter[5][6], C_out[6][5]);
PE_array[5][6].compute<5,6>(A_inter[5][6], M_MIN_inter[5][6], C_out[5][6], A_inter[5][7], M_MIN_inter[5][7], C_out[6][6]);
PE_array[5][7].compute<5,7>(A_inter[5][7], M_MIN_inter[5][7], C_out[5][7], A_inter[5][8], M_MIN_inter[5][8], C_out[6][7]);
PE_array[5][8].compute<5,8>(A_inter[5][8], M_MIN_inter[5][8], C_out[5][8], A_inter[5][9], M_MIN_inter[5][9], C_out[6][8]);
PE_array[5][9].compute<5,9>(A_inter[5][9], M_MIN_inter[5][9], C_out[5][9], A_inter[5][10], M_MIN_inter[5][10], C_out[6][9]);
PE_array[5][10].compute<5,10>(A_inter[5][10], M_MIN_inter[5][10], C_out[5][10], A_inter[5][11], M_MIN_inter[5][11], C_out[6][10]);

PE_array[6][0].compute<6,0>(     Ain_s[6],        M_MIN_s[6], C_out[6][0], A_inter[6][1], M_MIN_inter[6][1], C_out[7][0]);
PE_array[6][1].compute<6,1>(A_inter[6][1], M_MIN_inter[6][1], C_out[6][1], A_inter[6][2], M_MIN_inter[6][2], C_out[7][1]);
PE_array[6][2].compute<6,2>(A_inter[6][2], M_MIN_inter[6][2], C_out[6][2], A_inter[6][3], M_MIN_inter[6][3], C_out[7][2]);
PE_array[6][3].compute<6,3>(A_inter[6][3], M_MIN_inter[6][3], C_out[6][3], A_inter[6][4], M_MIN_inter[6][4], C_out[7][3]);
PE_array[6][4].compute<6,4>(A_inter[6][4], M_MIN_inter[6][4], C_out[6][4], A_inter[6][5], M_MIN_inter[6][5], C_out[7][4]);
PE_array[6][5].compute<6,5>(A_inter[6][5], M_MIN_inter[6][5], C_out[6][5], A_inter[6][6], M_MIN_inter[6][6], C_out[7][5]);
PE_array[6][6].compute<6,6>(A_inter[6][6], M_MIN_inter[6][6], C_out[6][6], A_inter[6][7], M_MIN_inter[6][7], C_out[7][6]);
PE_array[6][7].compute<6,7>(A_inter[6][7], M_MIN_inter[6][7], C_out[6][7], A_inter[6][8], M_MIN_inter[6][8], C_out[7][7]);
PE_array[6][8].compute<6,8>(A_inter[6][8], M_MIN_inter[6][8], C_out[6][8], A_inter[6][9], M_MIN_inter[6][9], C_out[7][8]);
PE_array[6][9].compute<6,9>(A_inter[6][9], M_MIN_inter[6][9], C_out[6][9], A_inter[6][10], M_MIN_inter[6][10], C_out[7][9]);
PE_array[6][10].compute<6,10>(A_inter[6][10], M_MIN_inter[6][10], C_out[6][10], A_inter[6][11], M_MIN_inter[6][11], C_out[7][10]);

PE_array[7][0].compute<7,0>(     Ain_s[7],        M_MIN_s[7], C_out[7][0], A_inter[7][1], M_MIN_inter[7][1], C_out[8][0]);
PE_array[7][1].compute<7,1>(A_inter[7][1], M_MIN_inter[7][1], C_out[7][1], A_inter[7][2], M_MIN_inter[7][2], C_out[8][1]);
PE_array[7][2].compute<7,2>(A_inter[7][2], M_MIN_inter[7][2], C_out[7][2], A_inter[7][3], M_MIN_inter[7][3], C_out[8][2]);
PE_array[7][3].compute<7,3>(A_inter[7][3], M_MIN_inter[7][3], C_out[7][3], A_inter[7][4], M_MIN_inter[7][4], C_out[8][3]);
PE_array[7][4].compute<7,4>(A_inter[7][4], M_MIN_inter[7][4], C_out[7][4], A_inter[7][5], M_MIN_inter[7][5], C_out[8][4]);
PE_array[7][5].compute<7,5>(A_inter[7][5], M_MIN_inter[7][5], C_out[7][5], A_inter[7][6], M_MIN_inter[7][6], C_out[8][5]);
PE_array[7][6].compute<7,6>(A_inter[7][6], M_MIN_inter[7][6], C_out[7][6], A_inter[7][7], M_MIN_inter[7][7], C_out[8][6]);
PE_array[7][7].compute<7,7>(A_inter[7][7], M_MIN_inter[7][7], C_out[7][7], A_inter[7][8], M_MIN_inter[7][8], C_out[8][7]);
PE_array[7][8].compute<7,8>(A_inter[7][8], M_MIN_inter[7][8], C_out[7][8], A_inter[7][9], M_MIN_inter[7][9], C_out[8][8]);
PE_array[7][9].compute<7,9>(A_inter[7][9], M_MIN_inter[7][9], C_out[7][9], A_inter[7][10], M_MIN_inter[7][10], C_out[8][9]);
PE_array[7][10].compute<7,10>(A_inter[7][10], M_MIN_inter[7][10], C_out[7][10], A_inter[7][11], M_MIN_inter[7][11], C_out[8][10]);

PE_array[8][0].compute<8,0>(     Ain_s[8],        M_MIN_s[8], C_out[8][0], A_inter[8][1], M_MIN_inter[8][1], C_out[9][0]);
PE_array[8][1].compute<8,1>(A_inter[8][1], M_MIN_inter[8][1], C_out[8][1], A_inter[8][2], M_MIN_inter[8][2], C_out[9][1]);
PE_array[8][2].compute<8,2>(A_inter[8][2], M_MIN_inter[8][2], C_out[8][2], A_inter[8][3], M_MIN_inter[8][3], C_out[9][2]);
PE_array[8][3].compute<8,3>(A_inter[8][3], M_MIN_inter[8][3], C_out[8][3], A_inter[8][4], M_MIN_inter[8][4], C_out[9][3]);
PE_array[8][4].compute<8,4>(A_inter[8][4], M_MIN_inter[8][4], C_out[8][4], A_inter[8][5], M_MIN_inter[8][5], C_out[9][4]);
PE_array[8][5].compute<8,5>(A_inter[8][5], M_MIN_inter[8][5], C_out[8][5], A_inter[8][6], M_MIN_inter[8][6], C_out[9][5]);
PE_array[8][6].compute<8,6>(A_inter[8][6], M_MIN_inter[8][6], C_out[8][6], A_inter[8][7], M_MIN_inter[8][7], C_out[9][6]);
PE_array[8][7].compute<8,7>(A_inter[8][7], M_MIN_inter[8][7], C_out[8][7], A_inter[8][8], M_MIN_inter[8][8], C_out[9][7]);
PE_array[8][8].compute<8,8>(A_inter[8][8], M_MIN_inter[8][8], C_out[8][8], A_inter[8][9], M_MIN_inter[8][9], C_out[9][8]);
PE_array[8][9].compute<8,9>(A_inter[8][9], M_MIN_inter[8][9], C_out[8][9], A_inter[8][10], M_MIN_inter[8][10], C_out[9][9]);
PE_array[8][10].compute<8,10>(A_inter[8][10], M_MIN_inter[8][10], C_out[8][10], A_inter[8][11], M_MIN_inter[8][11], C_out[9][10]);

PE_array[9][0].compute<9,0>(     Ain_s[9],        M_MIN_s[9], C_out[9][0], A_inter[9][1], M_MIN_inter[9][1], C_out[10][0]);
PE_array[9][1].compute<9,1>(A_inter[9][1], M_MIN_inter[9][1], C_out[9][1], A_inter[9][2], M_MIN_inter[9][2], C_out[10][1]);
PE_array[9][2].compute<9,2>(A_inter[9][2], M_MIN_inter[9][2], C_out[9][2], A_inter[9][3], M_MIN_inter[9][3], C_out[10][2]);
PE_array[9][3].compute<9,3>(A_inter[9][3], M_MIN_inter[9][3], C_out[9][3], A_inter[9][4], M_MIN_inter[9][4], C_out[10][3]);
PE_array[9][4].compute<9,4>(A_inter[9][4], M_MIN_inter[9][4], C_out[9][4], A_inter[9][5], M_MIN_inter[9][5], C_out[10][4]);
PE_array[9][5].compute<9,5>(A_inter[9][5], M_MIN_inter[9][5], C_out[9][5], A_inter[9][6], M_MIN_inter[9][6], C_out[10][5]);
PE_array[9][6].compute<9,6>(A_inter[9][6], M_MIN_inter[9][6], C_out[9][6], A_inter[9][7], M_MIN_inter[9][7], C_out[10][6]);
PE_array[9][7].compute<9,7>(A_inter[9][7], M_MIN_inter[9][7], C_out[9][7], A_inter[9][8], M_MIN_inter[9][8], C_out[10][7]);
PE_array[9][8].compute<9,8>(A_inter[9][8], M_MIN_inter[9][8], C_out[9][8], A_inter[9][9], M_MIN_inter[9][9], C_out[10][8]);
PE_array[9][9].compute<9,9>(A_inter[9][9], M_MIN_inter[9][9], C_out[9][9], A_inter[9][10], M_MIN_inter[9][10], C_out[10][9]);
PE_array[9][10].compute<9,10>(A_inter[9][10], M_MIN_inter[9][10], C_out[9][10], A_inter[9][11], M_MIN_inter[9][11], C_out[10][10]);

PE_array[10][0].compute<10,0>(     Ain_s[10],        M_MIN_s[10], C_out[10][0], A_inter[10][1], M_MIN_inter[10][1], C_out[11][0]);
PE_array[10][1].compute<10,1>(A_inter[10][1], M_MIN_inter[10][1], C_out[10][1], A_inter[10][2], M_MIN_inter[10][2], C_out[11][1]);
PE_array[10][2].compute<10,2>(A_inter[10][2], M_MIN_inter[10][2], C_out[10][2], A_inter[10][3], M_MIN_inter[10][3], C_out[11][2]);
PE_array[10][3].compute<10,3>(A_inter[10][3], M_MIN_inter[10][3], C_out[10][3], A_inter[10][4], M_MIN_inter[10][4], C_out[11][3]);
PE_array[10][4].compute<10,4>(A_inter[10][4], M_MIN_inter[10][4], C_out[10][4], A_inter[10][5], M_MIN_inter[10][5], C_out[11][4]);
PE_array[10][5].compute<10,5>(A_inter[10][5], M_MIN_inter[10][5], C_out[10][5], A_inter[10][6], M_MIN_inter[10][6], C_out[11][5]);
PE_array[10][6].compute<10,6>(A_inter[10][6], M_MIN_inter[10][6], C_out[10][6], A_inter[10][7], M_MIN_inter[10][7], C_out[11][6]);
PE_array[10][7].compute<10,7>(A_inter[10][7], M_MIN_inter[10][7], C_out[10][7], A_inter[10][8], M_MIN_inter[10][8], C_out[11][7]);
PE_array[10][8].compute<10,8>(A_inter[10][8], M_MIN_inter[10][8], C_out[10][8], A_inter[10][9], M_MIN_inter[10][9], C_out[11][8]);
PE_array[10][9].compute<10,9>(A_inter[10][9], M_MIN_inter[10][9], C_out[10][9], A_inter[10][10], M_MIN_inter[10][10], C_out[11][9]);
PE_array[10][10].compute<10,10>(A_inter[10][10], M_MIN_inter[10][10], C_out[10][10], A_inter[10][11], M_MIN_inter[10][11], C_out[11][10]);

PE_array[11][0].compute<11,0>(     Ain_s[11],        M_MIN_s[11], C_out[11][0], A_inter[11][1], M_MIN_inter[11][1], C_out[12][0]);
PE_array[11][1].compute<11,1>(A_inter[11][1], M_MIN_inter[11][1], C_out[11][1], A_inter[11][2], M_MIN_inter[11][2], C_out[12][1]);
PE_array[11][2].compute<11,2>(A_inter[11][2], M_MIN_inter[11][2], C_out[11][2], A_inter[11][3], M_MIN_inter[11][3], C_out[12][2]);
PE_array[11][3].compute<11,3>(A_inter[11][3], M_MIN_inter[11][3], C_out[11][3], A_inter[11][4], M_MIN_inter[11][4], C_out[12][3]);
PE_array[11][4].compute<11,4>(A_inter[11][4], M_MIN_inter[11][4], C_out[11][4], A_inter[11][5], M_MIN_inter[11][5], C_out[12][4]);
PE_array[11][5].compute<11,5>(A_inter[11][5], M_MIN_inter[11][5], C_out[11][5], A_inter[11][6], M_MIN_inter[11][6], C_out[12][5]);
PE_array[11][6].compute<11,6>(A_inter[11][6], M_MIN_inter[11][6], C_out[11][6], A_inter[11][7], M_MIN_inter[11][7], C_out[12][6]);
PE_array[11][7].compute<11,7>(A_inter[11][7], M_MIN_inter[11][7], C_out[11][7], A_inter[11][8], M_MIN_inter[11][8], C_out[12][7]);
PE_array[11][8].compute<11,8>(A_inter[11][8], M_MIN_inter[11][8], C_out[11][8], A_inter[11][9], M_MIN_inter[11][9], C_out[12][8]);
PE_array[11][9].compute<11,9>(A_inter[11][9], M_MIN_inter[11][9], C_out[11][9], A_inter[11][10], M_MIN_inter[11][10], C_out[12][9]);
PE_array[11][10].compute<11,10>(A_inter[11][10], M_MIN_inter[11][10], C_out[11][10], A_inter[11][11], M_MIN_inter[11][11], C_out[12][10]);

PE_array[12][0].compute<12,0>(     Ain_s[12],        M_MIN_s[12], C_out[12][0], A_inter[12][1], M_MIN_inter[12][1], Cout_s[0]);
PE_array[12][1].compute<12,1>(A_inter[12][1], M_MIN_inter[12][1], C_out[12][1], A_inter[12][2], M_MIN_inter[12][2], Cout_s[1]);
PE_array[12][2].compute<12,2>(A_inter[12][2], M_MIN_inter[12][2], C_out[12][2], A_inter[12][3], M_MIN_inter[12][3], Cout_s[2]);
PE_array[12][3].compute<12,3>(A_inter[12][3], M_MIN_inter[12][3], C_out[12][3], A_inter[12][4], M_MIN_inter[12][4], Cout_s[3]);
PE_array[12][4].compute<12,4>(A_inter[12][4], M_MIN_inter[12][4], C_out[12][4], A_inter[12][5], M_MIN_inter[12][5], Cout_s[4]);
PE_array[12][5].compute<12,5>(A_inter[12][5], M_MIN_inter[12][5], C_out[12][5], A_inter[12][6], M_MIN_inter[12][6], Cout_s[5]);
PE_array[12][6].compute<12,6>(A_inter[12][6], M_MIN_inter[12][6], C_out[12][6], A_inter[12][7], M_MIN_inter[12][7], Cout_s[6]);
PE_array[12][7].compute<12,7>(A_inter[12][7], M_MIN_inter[12][7], C_out[12][7], A_inter[12][8], M_MIN_inter[12][8], Cout_s[7]);
PE_array[12][8].compute<12,8>(A_inter[12][8], M_MIN_inter[12][8], C_out[12][8], A_inter[12][9], M_MIN_inter[12][9], Cout_s[8]);
PE_array[12][9].compute<12,9>(A_inter[12][9], M_MIN_inter[12][9], C_out[12][9], A_inter[12][10], M_MIN_inter[12][10], Cout_s[9]);
PE_array[12][10].compute<12,10>(A_inter[12][10], M_MIN_inter[12][10], C_out[12][10], A_inter[12][11], M_MIN_inter[12][11], Cout_s[10]);



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

