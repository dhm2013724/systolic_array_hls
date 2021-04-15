
#include "systolic_array.h"

////////////////////////////////////////////v12-iws_c syn ok  start////////////////////////////////////////
//typedef struct {
//	bool wb;
//	bool init;
//} wb_wrap_s;
//
//
//template<int MIN_R,int MID_C>
//void compute(hls::stream<int> &A_in, hls::stream<int> &B_in, hls::stream<int> &C_in,
//		hls::stream<int> &A_out, hls::stream<int> &B_out, hls::stream<int> &C_out, int M_MIN)
//{
//	int weight_local;
//	int tmp_buf[TILE_M];
//
//	int B_tmp;
//	B_in >> B_tmp;
//	weight_local = B_tmp;
////tranfer neighbor PE's weight
//	for(int k=0; k<SA_R-1-MIN_R; k++){
//#pragma HLS PIPELINE II=1
//		int tmp_in, tmp_out;
//		B_in  >> tmp_in;
//		tmp_out = tmp_in;
//		B_out << tmp_out;
//	}
//
//#pragma HLS INLINE off
//	for(int i = 0; i < M_MIN; i++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
//		int tmp, A_tmp;
//		A_in >> A_tmp;
//		tmp_buf[i] = A_tmp * weight_local;
//		A_out << A_tmp;
//	}
//
//	for(int i = 0; i < M_MIN; i++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
//		C_out << tmp_buf[i];
//	}
//
//	int trans_num = MIN_R*M_MIN;
//
//	for(int i = 0; i < trans_num; i++){
//#pragma HLS LOOP_TRIPCOUNT min=1 max=PE_TRANS_NUM_MAX
//#pragma HLS PIPELINE II=1
//		int tmp;
//		C_in >> tmp;
//		C_out << tmp;
//	}
//
//	return;
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//#pragma HLS INLINE off
//	int drain;
//	for(int k = 0; k<data_num; k++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
//		in >> drain;
//	}
//}
//
//void Writeback_SA_Cout(int C_local[TILE_M][SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb1_s)
//{
//	static int partial_sum[TILE_M][SA_C];
//#pragma HLS ARRAY_PARTITION variable=partial_sum complete dim=2
//	wb_wrap_s wbw_s;
//
//	wbw_s = wb1_s.read();
//	bool init = wbw_s.init;
//	bool wb = wbw_s.wb;
//
////	L1:for(int k=0; k<SA_R;k++){
////#pragma HLS LOOP_FLATTEN off
////		L2:for(int i=0; i<M_MIN;i++)
////		{
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
////#pragma HLS PIPELINE II=1
//////#pragma HLS DEPENDENCE variable=partial_sum inter false
////			for(int j=0; j<SA_C; j++)
////			{
////				int tmp, psum;
////				if((k==0)&&init)
////					psum = 0;
////				else
////					psum = partial_sum[i][j];
////				tmp = Cout_s[j].read();
////				partial_sum[i][j] = psum + tmp;
////			}
////		}
////	}
//
//	int k = 0;
//	while(k<SA_R){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
//		L2:for(int i=0; i<M_MIN;i++)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
////#pragma HLS DEPENDENCE variable=partial_sum inter false
//			for(int j=0; j<SA_C; j++)
//			{
//				int tmp, psum;
//				if((k==0)&&init)
//					psum = 0;
//				else
//					psum = partial_sum[i][j];
//				tmp = Cout_s[j].read();
//				partial_sum[i][j] = psum + tmp;
//			}
//		}
//		k++;
//	}
//
//	if(wb){
//		for(int i=0; i<M_MIN; i++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//				for(int j=0; j<N_MIN; j++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//			C_local[i][j] = partial_sum[i][j];
//				}
//		}
//	}
//}
//
//void Compute_SA(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN,
//		hls::stream<wb_wrap_s> &wb0_s, hls::stream<wb_wrap_s> &wb1_s)
//{
//	hls::stream<int> A_inter[SA_R][SA_C+1];
//#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_M
//#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_M
//	hls::stream<int> B_inter[SA_R+1][SA_C];
//#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_SA_R
//#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_SA_R
//	hls::stream<int> C_out[SA_R+1][SA_C];
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_COUT_S
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_COUT_S
//
//	FILL_B:{
////		PE_array[0][0].fillB<0,0>(Bin_s[0], B_inter[0+1][0]);
////		PE_array[0][1].fillB<0,1>(Bin_s[1], B_inter[0+1][1]);
////		PE_array[0][2].fillB<0,2>(Bin_s[2], B_inter[0+1][2]);
////		PE_array[0][3].fillB<0,3>(Bin_s[3], B_inter[0+1][3]);
////		PE_array[0][4].fillB<0,4>(Bin_s[4], B_inter[0+1][4]);
////		PE_array[0][5].fillB<0,5>(Bin_s[5], B_inter[0+1][5]);
////		PE_array[0][6].fillB<0,6>(Bin_s[6], B_inter[0+1][6]);
////		PE_array[0][7].fillB<0,7>(Bin_s[7], B_inter[0+1][7]);
////		PE_array[0][8].fillB<0,8>(Bin_s[8], B_inter[0+1][8]);
//
////		PE_array[1][0].fillB<1,0>(B_inter[1][0], B_inter[1+1][0]);
////		PE_array[1][1].fillB<1,1>(B_inter[1][1], B_inter[1+1][1]);
////		PE_array[1][2].fillB<1,2>(B_inter[1][2], B_inter[1+1][2]);
////		PE_array[1][3].fillB<1,3>(B_inter[1][3], B_inter[1+1][3]);
////		PE_array[1][4].fillB<1,4>(B_inter[1][4], B_inter[1+1][4]);
////		PE_array[1][5].fillB<1,5>(B_inter[1][5], B_inter[1+1][5]);
////		PE_array[1][6].fillB<1,6>(B_inter[1][6], B_inter[1+1][6]);
////		PE_array[1][7].fillB<1,7>(B_inter[1][7], B_inter[1+1][7]);
////		PE_array[1][8].fillB<1,8>(B_inter[1][8], B_inter[1+1][8]);
//
////		PE_array[2][0].fillB<2,0>(B_inter[2][0], B_inter[2+1][0]);
////		PE_array[2][1].fillB<2,1>(B_inter[2][1], B_inter[2+1][1]);
////		PE_array[2][2].fillB<2,2>(B_inter[2][2], B_inter[2+1][2]);
////		PE_array[2][3].fillB<2,3>(B_inter[2][3], B_inter[2+1][3]);
////		PE_array[2][4].fillB<2,4>(B_inter[2][4], B_inter[2+1][4]);
////		PE_array[2][5].fillB<2,5>(B_inter[2][5], B_inter[2+1][5]);
////		PE_array[2][6].fillB<2,6>(B_inter[2][6], B_inter[2+1][6]);
////		PE_array[2][7].fillB<2,7>(B_inter[2][7], B_inter[2+1][7]);
////		PE_array[2][8].fillB<2,8>(B_inter[2][8], B_inter[2+1][8]);
//
////		PE_array[3][0].fillB<3,0>(B_inter[3][0], B_inter[3+1][0]);
////		PE_array[3][1].fillB<3,1>(B_inter[3][1], B_inter[3+1][1]);
////		PE_array[3][2].fillB<3,2>(B_inter[3][2], B_inter[3+1][2]);
////		PE_array[3][3].fillB<3,3>(B_inter[3][3], B_inter[3+1][3]);
////		PE_array[3][4].fillB<3,4>(B_inter[3][4], B_inter[3+1][4]);
////		PE_array[3][5].fillB<3,5>(B_inter[3][5], B_inter[3+1][5]);
////		PE_array[3][6].fillB<3,6>(B_inter[3][6], B_inter[3+1][6]);
////		PE_array[3][7].fillB<3,7>(B_inter[3][7], B_inter[3+1][7]);
////		PE_array[3][8].fillB<3,8>(B_inter[3][8], B_inter[3+1][8]);
//
////		PE_array[4][0].fillB<4,0>(B_inter[4][0], B_inter[4+1][0]);
////		PE_array[4][1].fillB<4,1>(B_inter[4][1], B_inter[4+1][1]);
////		PE_array[4][2].fillB<4,2>(B_inter[4][2], B_inter[4+1][2]);
////		PE_array[4][3].fillB<4,3>(B_inter[4][3], B_inter[4+1][3]);
////		PE_array[4][4].fillB<4,4>(B_inter[4][4], B_inter[4+1][4]);
////		PE_array[4][5].fillB<4,5>(B_inter[4][5], B_inter[4+1][5]);
////		PE_array[4][6].fillB<4,6>(B_inter[4][6], B_inter[4+1][6]);
////		PE_array[4][7].fillB<4,7>(B_inter[4][7], B_inter[4+1][7]);
////		PE_array[4][8].fillB<4,8>(B_inter[4][8], B_inter[4+1][8]);
//
////		PE_array[5][0].fillB<5,0>(B_inter[5][0], B_inter[5+1][0]);
////		PE_array[5][1].fillB<5,1>(B_inter[5][1], B_inter[5+1][1]);
////		PE_array[5][2].fillB<5,2>(B_inter[5][2], B_inter[5+1][2]);
////		PE_array[5][3].fillB<5,3>(B_inter[5][3], B_inter[5+1][3]);
////		PE_array[5][4].fillB<5,4>(B_inter[5][4], B_inter[5+1][4]);
////		PE_array[5][5].fillB<5,5>(B_inter[5][5], B_inter[5+1][5]);
////		PE_array[5][6].fillB<5,6>(B_inter[5][6], B_inter[5+1][6]);
////		PE_array[5][7].fillB<5,7>(B_inter[5][7], B_inter[5+1][7]);
////		PE_array[5][8].fillB<5,8>(B_inter[5][8], B_inter[5+1][8]);
//
////		PE_array[6][0].fillB<6,0>(B_inter[6][0], B_inter[6+1][0]);
////		PE_array[6][1].fillB<6,1>(B_inter[6][1], B_inter[6+1][1]);
////		PE_array[6][2].fillB<6,2>(B_inter[6][2], B_inter[6+1][2]);
////		PE_array[6][3].fillB<6,3>(B_inter[6][3], B_inter[6+1][3]);
////		PE_array[6][4].fillB<6,4>(B_inter[6][4], B_inter[6+1][4]);
////		PE_array[6][5].fillB<6,5>(B_inter[6][5], B_inter[6+1][5]);
////		PE_array[6][6].fillB<6,6>(B_inter[6][6], B_inter[6+1][6]);
////		PE_array[6][7].fillB<6,7>(B_inter[6][7], B_inter[6+1][7]);
////		PE_array[6][8].fillB<6,8>(B_inter[6][8], B_inter[6+1][8]);
//	}
//
//	Compute_SA_Loop:{
//		compute<0,0>(     Ain_s[0], Bin_s[0], C_out[0][0], A_inter[0][0+1], B_inter[0+1][0], C_out[0+1][0], M_MIN);
//		compute<0,1>(A_inter[0][1], Bin_s[1], C_out[0][1], A_inter[0][1+1], B_inter[0+1][1], C_out[0+1][1], M_MIN);
//		compute<0,2>(A_inter[0][2], Bin_s[2], C_out[0][2], A_inter[0][2+1], B_inter[0+1][2], C_out[0+1][2], M_MIN);
//		compute<0,3>(A_inter[0][3], Bin_s[3], C_out[0][3], A_inter[0][3+1], B_inter[0+1][3], C_out[0+1][3], M_MIN);
//		compute<0,4>(A_inter[0][4], Bin_s[4], C_out[0][4], A_inter[0][4+1], B_inter[0+1][4], C_out[0+1][4], M_MIN);
//		compute<0,5>(A_inter[0][5], Bin_s[5], C_out[0][5], A_inter[0][5+1], B_inter[0+1][5], C_out[0+1][5], M_MIN);
//		compute<0,6>(A_inter[0][6], Bin_s[6], C_out[0][6], A_inter[0][6+1], B_inter[0+1][6], C_out[0+1][6], M_MIN);
//		compute<0,7>(A_inter[0][7], Bin_s[7], C_out[0][7], A_inter[0][7+1], B_inter[0+1][7], C_out[0+1][7], M_MIN);
//		compute<0,8>(A_inter[0][8], Bin_s[8], C_out[0][8], A_inter[0][8+1], B_inter[0+1][8], C_out[0+1][8], M_MIN);
//
//		compute<1,0>(     Ain_s[1], B_inter[1][0], C_out[1][0], A_inter[1][0+1], B_inter[1+1][0], C_out[1+1][0], M_MIN);
//		compute<1,1>(A_inter[1][1], B_inter[1][1], C_out[1][1], A_inter[1][1+1], B_inter[1+1][1], C_out[1+1][1], M_MIN);
//		compute<1,2>(A_inter[1][2], B_inter[1][2], C_out[1][2], A_inter[1][2+1], B_inter[1+1][2], C_out[1+1][2], M_MIN);
//		compute<1,3>(A_inter[1][3], B_inter[1][3], C_out[1][3], A_inter[1][3+1], B_inter[1+1][3], C_out[1+1][3], M_MIN);
//		compute<1,4>(A_inter[1][4], B_inter[1][4], C_out[1][4], A_inter[1][4+1], B_inter[1+1][4], C_out[1+1][4], M_MIN);
//		compute<1,5>(A_inter[1][5], B_inter[1][5], C_out[1][5], A_inter[1][5+1], B_inter[1+1][5], C_out[1+1][5], M_MIN);
//		compute<1,6>(A_inter[1][6], B_inter[1][6], C_out[1][6], A_inter[1][6+1], B_inter[1+1][6], C_out[1+1][6], M_MIN);
//		compute<1,7>(A_inter[1][7], B_inter[1][7], C_out[1][7], A_inter[1][7+1], B_inter[1+1][7], C_out[1+1][7], M_MIN);
//		compute<1,8>(A_inter[1][8], B_inter[1][8], C_out[1][8], A_inter[1][8+1], B_inter[1+1][8], C_out[1+1][8], M_MIN);
//
//		compute<2,0>(     Ain_s[2], B_inter[2][0], C_out[2][0], A_inter[2][0+1], B_inter[2+1][0], C_out[2+1][0], M_MIN);
//		compute<2,1>(A_inter[2][1], B_inter[2][1], C_out[2][1], A_inter[2][1+1], B_inter[2+1][1], C_out[2+1][1], M_MIN);
//		compute<2,2>(A_inter[2][2], B_inter[2][2], C_out[2][2], A_inter[2][2+1], B_inter[2+1][2], C_out[2+1][2], M_MIN);
//		compute<2,3>(A_inter[2][3], B_inter[2][3], C_out[2][3], A_inter[2][3+1], B_inter[2+1][3], C_out[2+1][3], M_MIN);
//		compute<2,4>(A_inter[2][4], B_inter[2][4], C_out[2][4], A_inter[2][4+1], B_inter[2+1][4], C_out[2+1][4], M_MIN);
//		compute<2,5>(A_inter[2][5], B_inter[2][5], C_out[2][5], A_inter[2][5+1], B_inter[2+1][5], C_out[2+1][5], M_MIN);
//		compute<2,6>(A_inter[2][6], B_inter[2][6], C_out[2][6], A_inter[2][6+1], B_inter[2+1][6], C_out[2+1][6], M_MIN);
//		compute<2,7>(A_inter[2][7], B_inter[2][7], C_out[2][7], A_inter[2][7+1], B_inter[2+1][7], C_out[2+1][7], M_MIN);
//		compute<2,8>(A_inter[2][8], B_inter[2][8], C_out[2][8], A_inter[2][8+1], B_inter[2+1][8], C_out[2+1][8], M_MIN);
//
//		compute<3,0>(     Ain_s[3], B_inter[3][0], C_out[3][0], A_inter[3][0+1], B_inter[3+1][0], C_out[3+1][0], M_MIN);
//		compute<3,1>(A_inter[3][1], B_inter[3][1], C_out[3][1], A_inter[3][1+1], B_inter[3+1][1], C_out[3+1][1], M_MIN);
//		compute<3,2>(A_inter[3][2], B_inter[3][2], C_out[3][2], A_inter[3][2+1], B_inter[3+1][2], C_out[3+1][2], M_MIN);
//		compute<3,3>(A_inter[3][3], B_inter[3][3], C_out[3][3], A_inter[3][3+1], B_inter[3+1][3], C_out[3+1][3], M_MIN);
//		compute<3,4>(A_inter[3][4], B_inter[3][4], C_out[3][4], A_inter[3][4+1], B_inter[3+1][4], C_out[3+1][4], M_MIN);
//		compute<3,5>(A_inter[3][5], B_inter[3][5], C_out[3][5], A_inter[3][5+1], B_inter[3+1][5], C_out[3+1][5], M_MIN);
//		compute<3,6>(A_inter[3][6], B_inter[3][6], C_out[3][6], A_inter[3][6+1], B_inter[3+1][6], C_out[3+1][6], M_MIN);
//		compute<3,7>(A_inter[3][7], B_inter[3][7], C_out[3][7], A_inter[3][7+1], B_inter[3+1][7], C_out[3+1][7], M_MIN);
//		compute<3,8>(A_inter[3][8], B_inter[3][8], C_out[3][8], A_inter[3][8+1], B_inter[3+1][8], C_out[3+1][8], M_MIN);
//
//		compute<4,0>(     Ain_s[4], B_inter[4][0], C_out[4][0], A_inter[4][0+1], B_inter[4+1][0], C_out[4+1][0], M_MIN);
//		compute<4,1>(A_inter[4][1], B_inter[4][1], C_out[4][1], A_inter[4][1+1], B_inter[4+1][1], C_out[4+1][1], M_MIN);
//		compute<4,2>(A_inter[4][2], B_inter[4][2], C_out[4][2], A_inter[4][2+1], B_inter[4+1][2], C_out[4+1][2], M_MIN);
//		compute<4,3>(A_inter[4][3], B_inter[4][3], C_out[4][3], A_inter[4][3+1], B_inter[4+1][3], C_out[4+1][3], M_MIN);
//		compute<4,4>(A_inter[4][4], B_inter[4][4], C_out[4][4], A_inter[4][4+1], B_inter[4+1][4], C_out[4+1][4], M_MIN);
//		compute<4,5>(A_inter[4][5], B_inter[4][5], C_out[4][5], A_inter[4][5+1], B_inter[4+1][5], C_out[4+1][5], M_MIN);
//		compute<4,6>(A_inter[4][6], B_inter[4][6], C_out[4][6], A_inter[4][6+1], B_inter[4+1][6], C_out[4+1][6], M_MIN);
//		compute<4,7>(A_inter[4][7], B_inter[4][7], C_out[4][7], A_inter[4][7+1], B_inter[4+1][7], C_out[4+1][7], M_MIN);
//		compute<4,8>(A_inter[4][8], B_inter[4][8], C_out[4][8], A_inter[4][8+1], B_inter[4+1][8], C_out[4+1][8], M_MIN);
//
//		compute<5,0>(     Ain_s[5], B_inter[5][0], C_out[5][0], A_inter[5][0+1], B_inter[5+1][0], C_out[5+1][0], M_MIN);
//		compute<5,1>(A_inter[5][1], B_inter[5][1], C_out[5][1], A_inter[5][1+1], B_inter[5+1][1], C_out[5+1][1], M_MIN);
//		compute<5,2>(A_inter[5][2], B_inter[5][2], C_out[5][2], A_inter[5][2+1], B_inter[5+1][2], C_out[5+1][2], M_MIN);
//		compute<5,3>(A_inter[5][3], B_inter[5][3], C_out[5][3], A_inter[5][3+1], B_inter[5+1][3], C_out[5+1][3], M_MIN);
//		compute<5,4>(A_inter[5][4], B_inter[5][4], C_out[5][4], A_inter[5][4+1], B_inter[5+1][4], C_out[5+1][4], M_MIN);
//		compute<5,5>(A_inter[5][5], B_inter[5][5], C_out[5][5], A_inter[5][5+1], B_inter[5+1][5], C_out[5+1][5], M_MIN);
//		compute<5,6>(A_inter[5][6], B_inter[5][6], C_out[5][6], A_inter[5][6+1], B_inter[5+1][6], C_out[5+1][6], M_MIN);
//		compute<5,7>(A_inter[5][7], B_inter[5][7], C_out[5][7], A_inter[5][7+1], B_inter[5+1][7], C_out[5+1][7], M_MIN);
//		compute<5,8>(A_inter[5][8], B_inter[5][8], C_out[5][8], A_inter[5][8+1], B_inter[5+1][8], C_out[5+1][8], M_MIN);
//
//		compute<6,0>(     Ain_s[6], B_inter[6][0], C_out[6][0], A_inter[6][0+1], B_inter[6+1][0], Cout_s[0], M_MIN);
//		compute<6,1>(A_inter[6][1], B_inter[6][1], C_out[6][1], A_inter[6][1+1], B_inter[6+1][1], Cout_s[1], M_MIN);
//		compute<6,2>(A_inter[6][2], B_inter[6][2], C_out[6][2], A_inter[6][2+1], B_inter[6+1][2], Cout_s[2], M_MIN);
//		compute<6,3>(A_inter[6][3], B_inter[6][3], C_out[6][3], A_inter[6][3+1], B_inter[6+1][3], Cout_s[3], M_MIN);
//		compute<6,4>(A_inter[6][4], B_inter[6][4], C_out[6][4], A_inter[6][4+1], B_inter[6+1][4], Cout_s[4], M_MIN);
//		compute<6,5>(A_inter[6][5], B_inter[6][5], C_out[6][5], A_inter[6][5+1], B_inter[6+1][5], Cout_s[5], M_MIN);
//		compute<6,6>(A_inter[6][6], B_inter[6][6], C_out[6][6], A_inter[6][6+1], B_inter[6+1][6], Cout_s[6], M_MIN);
//		compute<6,7>(A_inter[6][7], B_inter[6][7], C_out[6][7], A_inter[6][7+1], B_inter[6+1][7], Cout_s[7], M_MIN);
//		compute<6,8>(A_inter[6][8], B_inter[6][8], C_out[6][8], A_inter[6][8+1], B_inter[6+1][8], Cout_s[8], M_MIN);
//	}
//
//	DRAIN_A:for(int i=0; i<SA_R; i++)
//	{
//#pragma HLS UNROLL
//		Drain(A_inter[i][SA_C], M_MIN);
//	}
//
//	wb_wrap_s wbw_s;
//	wbw_s = wb0_s.read();
//	wb1_s.write(wbw_s);
//}
//
//void Load_A(hls::stream<int> Ain_s[SA_R], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<SA_R; k++)
//			{
//#pragma HLS PIPELINE II=1
//				int tmp_in;
//				if(k<K_MIN)
//					tmp_in = A[base_offset + i*K_len + k];
//				else
//					tmp_in = 0;
//				Ain_s[k] << tmp_in;
//			}
//}
//
//void Load_B(hls::stream<int> Bin_s[SA_C], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<SA_R; i++)
//			Loop_N:for(int k=0; k<SA_C; k++)
//			{
//#pragma HLS PIPELINE II=1
//				int tmp_in;
//				bool k_en = i<K_MIN;
//				bool n_en = k<N_MIN;
//				bool ld_en = k_en && n_en;
//				if(ld_en)
//					tmp_in = B[base_offset + i*N_len + k];
//				else
//					tmp_in = 0;
//				Bin_s[k] << tmp_in;
//			}
//}
//
//void Store_C(int C_local[TILE_M][SA_C], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void Load_AB_wrapper(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], int *A, int *B, int i, int j, bool k_init,
//		int K, int N, int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb0_s){
//
//	static int k;
//	if(k_init){
//		k = 0;
//	}else{
//		k += SA_R;
//	}
//
//	int K_MIN = MIN(SA_R, K-k);
//
//	Load_A(Ain_s, A, i, k, K, M_MIN, K_MIN);
//	Load_B(Bin_s, B, k, j, N, K_MIN, N_MIN);
//
//	wb_wrap_s wbw_s0;
//	wbw_s0.wb = ((k+K_MIN) == K);
//	wbw_s0.init = (k == 0);
//	wb0_s.write(wbw_s0);
//}
//
//void DATAFLOW_SA_LCW(int C_local[TILE_M][SA_C], int *A, int *B, int M, int N, int K, int kloops,
//		int i, int j, int M_MIN, int N_MIN)
//{
//	hls::stream<int> Ain_s[SA_R];
//#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_M
//	hls::stream<int> Bin_s[SA_C];
//#pragma HLS STREAM variable=Bin_s depth=DEPTH_SA_R
//	hls::stream<int> Cout_s[SA_C];
//#pragma HLS STREAM variable=Cout_s depth=DEPTH_COUT_S
//
//	hls::stream<wb_wrap_s> wb0_s;
//	hls::stream<wb_wrap_s> wb1_s;
//
//	Loop_K:for(int k=0; k<kloops; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
//#pragma HLS DATAFLOW
//		Load_AB_wrapper( Ain_s, Bin_s, A, B, i, j, k==0, K, N, M_MIN, N_MIN, wb0_s);
//
//		Compute_SA(Ain_s, Bin_s, Cout_s, M_MIN, wb0_s, wb1_s);
//
//		Writeback_SA_Cout( C_local, Cout_s, M_MIN, N_MIN, wb1_s);
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
//	static int C_local[TILE_M][SA_C];
////#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN;
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= SA_C)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
//			N_MIN = MIN(SA_C, N-j);
//			DATAFLOW_SA_LCW( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v12-iws_c syn ok end////////////////////////////////////////


//////////////////////////////////////////v13-iws_c syn ok  start////////////////////////////////////////
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

//	L1:for(int k=0; k<SA_R;k++){
//#pragma HLS LOOP_FLATTEN off
//		L2:for(int i=0; i<M_MIN;i++)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
////#pragma HLS DEPENDENCE variable=partial_sum inter false
//			for(int j=0; j<SA_C; j++)
//			{
//				int tmp, psum;
//				if((k==0)&&init)
//					psum = 0;
//				else
//					psum = partial_sum[i][j];
//				tmp = Cout_s[j].read();
//				partial_sum[i][j] = psum + tmp;
//			}
//		}
//	}

	int k = 0;
	while(k<SA_R){
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
		L2:for(int i=0; i<M_MIN;i++)
		{
DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=partial_sum inter false
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
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_COUT_S
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_COUT_S

	hls::stream<int> M_MIN_inter[SA_R][SA_C+1];
#pragma HLS STREAM variable=M_MIN_inter dim=1
#pragma HLS STREAM variable=M_MIN_inter dim=2

	static PE_cls PE_array[SA_R][SA_C];
#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=1
#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=2

	FILL_B:{
		PE_array[0][0].fillB<0,0>(Bin_s[0], B_inter[0+1][0]);
		PE_array[0][1].fillB<0,1>(Bin_s[1], B_inter[0+1][1]);
		PE_array[0][2].fillB<0,2>(Bin_s[2], B_inter[0+1][2]);
		PE_array[0][3].fillB<0,3>(Bin_s[3], B_inter[0+1][3]);
		PE_array[0][4].fillB<0,4>(Bin_s[4], B_inter[0+1][4]);
		PE_array[0][5].fillB<0,5>(Bin_s[5], B_inter[0+1][5]);
		PE_array[0][6].fillB<0,6>(Bin_s[6], B_inter[0+1][6]);
		PE_array[0][7].fillB<0,7>(Bin_s[7], B_inter[0+1][7]);
		PE_array[0][8].fillB<0,8>(Bin_s[8], B_inter[0+1][8]);

		PE_array[1][0].fillB<1,0>(B_inter[1][0], B_inter[1+1][0]);
		PE_array[1][1].fillB<1,1>(B_inter[1][1], B_inter[1+1][1]);
		PE_array[1][2].fillB<1,2>(B_inter[1][2], B_inter[1+1][2]);
		PE_array[1][3].fillB<1,3>(B_inter[1][3], B_inter[1+1][3]);
		PE_array[1][4].fillB<1,4>(B_inter[1][4], B_inter[1+1][4]);
		PE_array[1][5].fillB<1,5>(B_inter[1][5], B_inter[1+1][5]);
		PE_array[1][6].fillB<1,6>(B_inter[1][6], B_inter[1+1][6]);
		PE_array[1][7].fillB<1,7>(B_inter[1][7], B_inter[1+1][7]);
		PE_array[1][8].fillB<1,8>(B_inter[1][8], B_inter[1+1][8]);

		PE_array[2][0].fillB<2,0>(B_inter[2][0], B_inter[2+1][0]);
		PE_array[2][1].fillB<2,1>(B_inter[2][1], B_inter[2+1][1]);
		PE_array[2][2].fillB<2,2>(B_inter[2][2], B_inter[2+1][2]);
		PE_array[2][3].fillB<2,3>(B_inter[2][3], B_inter[2+1][3]);
		PE_array[2][4].fillB<2,4>(B_inter[2][4], B_inter[2+1][4]);
		PE_array[2][5].fillB<2,5>(B_inter[2][5], B_inter[2+1][5]);
		PE_array[2][6].fillB<2,6>(B_inter[2][6], B_inter[2+1][6]);
		PE_array[2][7].fillB<2,7>(B_inter[2][7], B_inter[2+1][7]);
		PE_array[2][8].fillB<2,8>(B_inter[2][8], B_inter[2+1][8]);

		PE_array[3][0].fillB<3,0>(B_inter[3][0], B_inter[3+1][0]);
		PE_array[3][1].fillB<3,1>(B_inter[3][1], B_inter[3+1][1]);
		PE_array[3][2].fillB<3,2>(B_inter[3][2], B_inter[3+1][2]);
		PE_array[3][3].fillB<3,3>(B_inter[3][3], B_inter[3+1][3]);
		PE_array[3][4].fillB<3,4>(B_inter[3][4], B_inter[3+1][4]);
		PE_array[3][5].fillB<3,5>(B_inter[3][5], B_inter[3+1][5]);
		PE_array[3][6].fillB<3,6>(B_inter[3][6], B_inter[3+1][6]);
		PE_array[3][7].fillB<3,7>(B_inter[3][7], B_inter[3+1][7]);
		PE_array[3][8].fillB<3,8>(B_inter[3][8], B_inter[3+1][8]);

		PE_array[4][0].fillB<4,0>(B_inter[4][0], B_inter[4+1][0]);
		PE_array[4][1].fillB<4,1>(B_inter[4][1], B_inter[4+1][1]);
		PE_array[4][2].fillB<4,2>(B_inter[4][2], B_inter[4+1][2]);
		PE_array[4][3].fillB<4,3>(B_inter[4][3], B_inter[4+1][3]);
		PE_array[4][4].fillB<4,4>(B_inter[4][4], B_inter[4+1][4]);
		PE_array[4][5].fillB<4,5>(B_inter[4][5], B_inter[4+1][5]);
		PE_array[4][6].fillB<4,6>(B_inter[4][6], B_inter[4+1][6]);
		PE_array[4][7].fillB<4,7>(B_inter[4][7], B_inter[4+1][7]);
		PE_array[4][8].fillB<4,8>(B_inter[4][8], B_inter[4+1][8]);

		PE_array[5][0].fillB<5,0>(B_inter[5][0], B_inter[5+1][0]);
		PE_array[5][1].fillB<5,1>(B_inter[5][1], B_inter[5+1][1]);
		PE_array[5][2].fillB<5,2>(B_inter[5][2], B_inter[5+1][2]);
		PE_array[5][3].fillB<5,3>(B_inter[5][3], B_inter[5+1][3]);
		PE_array[5][4].fillB<5,4>(B_inter[5][4], B_inter[5+1][4]);
		PE_array[5][5].fillB<5,5>(B_inter[5][5], B_inter[5+1][5]);
		PE_array[5][6].fillB<5,6>(B_inter[5][6], B_inter[5+1][6]);
		PE_array[5][7].fillB<5,7>(B_inter[5][7], B_inter[5+1][7]);
		PE_array[5][8].fillB<5,8>(B_inter[5][8], B_inter[5+1][8]);

		PE_array[6][0].fillB<6,0>(B_inter[6][0], B_inter[6+1][0]);
		PE_array[6][1].fillB<6,1>(B_inter[6][1], B_inter[6+1][1]);
		PE_array[6][2].fillB<6,2>(B_inter[6][2], B_inter[6+1][2]);
		PE_array[6][3].fillB<6,3>(B_inter[6][3], B_inter[6+1][3]);
		PE_array[6][4].fillB<6,4>(B_inter[6][4], B_inter[6+1][4]);
		PE_array[6][5].fillB<6,5>(B_inter[6][5], B_inter[6+1][5]);
		PE_array[6][6].fillB<6,6>(B_inter[6][6], B_inter[6+1][6]);
		PE_array[6][7].fillB<6,7>(B_inter[6][7], B_inter[6+1][7]);
		PE_array[6][8].fillB<6,8>(B_inter[6][8], B_inter[6+1][8]);
	}

	Compute_SA_Loop:{
		PE_array[0][0].compute<0,0>(     Ain_s[0],        M_MIN_s[0], C_out[0][0], A_inter[0][0+1], M_MIN_inter[0][0+1], C_out[0+1][0]);
		PE_array[0][1].compute<0,1>(A_inter[0][1], M_MIN_inter[0][1], C_out[0][1], A_inter[0][1+1], M_MIN_inter[0][1+1], C_out[0+1][1]);
		PE_array[0][2].compute<0,2>(A_inter[0][2], M_MIN_inter[0][2], C_out[0][2], A_inter[0][2+1], M_MIN_inter[0][2+1], C_out[0+1][2]);
		PE_array[0][3].compute<0,3>(A_inter[0][3], M_MIN_inter[0][3], C_out[0][3], A_inter[0][3+1], M_MIN_inter[0][3+1], C_out[0+1][3]);
		PE_array[0][4].compute<0,4>(A_inter[0][4], M_MIN_inter[0][4], C_out[0][4], A_inter[0][4+1], M_MIN_inter[0][4+1], C_out[0+1][4]);
		PE_array[0][5].compute<0,5>(A_inter[0][5], M_MIN_inter[0][5], C_out[0][5], A_inter[0][5+1], M_MIN_inter[0][5+1], C_out[0+1][5]);
		PE_array[0][6].compute<0,6>(A_inter[0][6], M_MIN_inter[0][6], C_out[0][6], A_inter[0][6+1], M_MIN_inter[0][6+1], C_out[0+1][6]);
		PE_array[0][7].compute<0,7>(A_inter[0][7], M_MIN_inter[0][7], C_out[0][7], A_inter[0][7+1], M_MIN_inter[0][7+1], C_out[0+1][7]);
		PE_array[0][8].compute<0,8>(A_inter[0][8], M_MIN_inter[0][8], C_out[0][8], A_inter[0][8+1], M_MIN_inter[0][8+1], C_out[0+1][8]);

		PE_array[1][0].compute<1,0>(     Ain_s[1],        M_MIN_s[1], C_out[1][0], A_inter[1][0+1], M_MIN_inter[1][0+1], C_out[1+1][0]);
		PE_array[1][1].compute<1,1>(A_inter[1][1], M_MIN_inter[1][1], C_out[1][1], A_inter[1][1+1], M_MIN_inter[1][1+1], C_out[1+1][1]);
		PE_array[1][2].compute<1,2>(A_inter[1][2], M_MIN_inter[1][2], C_out[1][2], A_inter[1][2+1], M_MIN_inter[1][2+1], C_out[1+1][2]);
		PE_array[1][3].compute<1,3>(A_inter[1][3], M_MIN_inter[1][3], C_out[1][3], A_inter[1][3+1], M_MIN_inter[1][3+1], C_out[1+1][3]);
		PE_array[1][4].compute<1,4>(A_inter[1][4], M_MIN_inter[1][4], C_out[1][4], A_inter[1][4+1], M_MIN_inter[1][4+1], C_out[1+1][4]);
		PE_array[1][5].compute<1,5>(A_inter[1][5], M_MIN_inter[1][5], C_out[1][5], A_inter[1][5+1], M_MIN_inter[1][5+1], C_out[1+1][5]);
		PE_array[1][6].compute<1,6>(A_inter[1][6], M_MIN_inter[1][6], C_out[1][6], A_inter[1][6+1], M_MIN_inter[1][6+1], C_out[1+1][6]);
		PE_array[1][7].compute<1,7>(A_inter[1][7], M_MIN_inter[1][7], C_out[1][7], A_inter[1][7+1], M_MIN_inter[1][7+1], C_out[1+1][7]);
		PE_array[1][8].compute<1,8>(A_inter[1][8], M_MIN_inter[1][8], C_out[1][8], A_inter[1][8+1], M_MIN_inter[1][8+1], C_out[1+1][8]);

		PE_array[2][0].compute<2,0>(     Ain_s[2],        M_MIN_s[2], C_out[2][0], A_inter[2][0+1], M_MIN_inter[2][0+1], C_out[2+1][0]);
		PE_array[2][1].compute<2,1>(A_inter[2][1], M_MIN_inter[2][1], C_out[2][1], A_inter[2][1+1], M_MIN_inter[2][1+1], C_out[2+1][1]);
		PE_array[2][2].compute<2,2>(A_inter[2][2], M_MIN_inter[2][2], C_out[2][2], A_inter[2][2+1], M_MIN_inter[2][2+1], C_out[2+1][2]);
		PE_array[2][3].compute<2,3>(A_inter[2][3], M_MIN_inter[2][3], C_out[2][3], A_inter[2][3+1], M_MIN_inter[2][3+1], C_out[2+1][3]);
		PE_array[2][4].compute<2,4>(A_inter[2][4], M_MIN_inter[2][4], C_out[2][4], A_inter[2][4+1], M_MIN_inter[2][4+1], C_out[2+1][4]);
		PE_array[2][5].compute<2,5>(A_inter[2][5], M_MIN_inter[2][5], C_out[2][5], A_inter[2][5+1], M_MIN_inter[2][5+1], C_out[2+1][5]);
		PE_array[2][6].compute<2,6>(A_inter[2][6], M_MIN_inter[2][6], C_out[2][6], A_inter[2][6+1], M_MIN_inter[2][6+1], C_out[2+1][6]);
		PE_array[2][7].compute<2,7>(A_inter[2][7], M_MIN_inter[2][7], C_out[2][7], A_inter[2][7+1], M_MIN_inter[2][7+1], C_out[2+1][7]);
		PE_array[2][8].compute<2,8>(A_inter[2][8], M_MIN_inter[2][8], C_out[2][8], A_inter[2][8+1], M_MIN_inter[2][8+1], C_out[2+1][8]);

		PE_array[3][0].compute<3,0>(     Ain_s[3],        M_MIN_s[3], C_out[3][0], A_inter[3][0+1], M_MIN_inter[3][0+1], C_out[3+1][0]);
		PE_array[3][1].compute<3,1>(A_inter[3][1], M_MIN_inter[3][1], C_out[3][1], A_inter[3][1+1], M_MIN_inter[3][1+1], C_out[3+1][1]);
		PE_array[3][2].compute<3,2>(A_inter[3][2], M_MIN_inter[3][2], C_out[3][2], A_inter[3][2+1], M_MIN_inter[3][2+1], C_out[3+1][2]);
		PE_array[3][3].compute<3,3>(A_inter[3][3], M_MIN_inter[3][3], C_out[3][3], A_inter[3][3+1], M_MIN_inter[3][3+1], C_out[3+1][3]);
		PE_array[3][4].compute<3,4>(A_inter[3][4], M_MIN_inter[3][4], C_out[3][4], A_inter[3][4+1], M_MIN_inter[3][4+1], C_out[3+1][4]);
		PE_array[3][5].compute<3,5>(A_inter[3][5], M_MIN_inter[3][5], C_out[3][5], A_inter[3][5+1], M_MIN_inter[3][5+1], C_out[3+1][5]);
		PE_array[3][6].compute<3,6>(A_inter[3][6], M_MIN_inter[3][6], C_out[3][6], A_inter[3][6+1], M_MIN_inter[3][6+1], C_out[3+1][6]);
		PE_array[3][7].compute<3,7>(A_inter[3][7], M_MIN_inter[3][7], C_out[3][7], A_inter[3][7+1], M_MIN_inter[3][7+1], C_out[3+1][7]);
		PE_array[3][8].compute<3,8>(A_inter[3][8], M_MIN_inter[3][8], C_out[3][8], A_inter[3][8+1], M_MIN_inter[3][8+1], C_out[3+1][8]);

		PE_array[4][0].compute<4,0>(     Ain_s[4],        M_MIN_s[4], C_out[4][0], A_inter[4][0+1], M_MIN_inter[4][0+1], C_out[4+1][0]);
		PE_array[4][1].compute<4,1>(A_inter[4][1], M_MIN_inter[4][1], C_out[4][1], A_inter[4][1+1], M_MIN_inter[4][1+1], C_out[4+1][1]);
		PE_array[4][2].compute<4,2>(A_inter[4][2], M_MIN_inter[4][2], C_out[4][2], A_inter[4][2+1], M_MIN_inter[4][2+1], C_out[4+1][2]);
		PE_array[4][3].compute<4,3>(A_inter[4][3], M_MIN_inter[4][3], C_out[4][3], A_inter[4][3+1], M_MIN_inter[4][3+1], C_out[4+1][3]);
		PE_array[4][4].compute<4,4>(A_inter[4][4], M_MIN_inter[4][4], C_out[4][4], A_inter[4][4+1], M_MIN_inter[4][4+1], C_out[4+1][4]);
		PE_array[4][5].compute<4,5>(A_inter[4][5], M_MIN_inter[4][5], C_out[4][5], A_inter[4][5+1], M_MIN_inter[4][5+1], C_out[4+1][5]);
		PE_array[4][6].compute<4,6>(A_inter[4][6], M_MIN_inter[4][6], C_out[4][6], A_inter[4][6+1], M_MIN_inter[4][6+1], C_out[4+1][6]);
		PE_array[4][7].compute<4,7>(A_inter[4][7], M_MIN_inter[4][7], C_out[4][7], A_inter[4][7+1], M_MIN_inter[4][7+1], C_out[4+1][7]);
		PE_array[4][8].compute<4,8>(A_inter[4][8], M_MIN_inter[4][8], C_out[4][8], A_inter[4][8+1], M_MIN_inter[4][8+1], C_out[4+1][8]);

		PE_array[5][0].compute<5,0>(     Ain_s[5],        M_MIN_s[5], C_out[5][0], A_inter[5][0+1], M_MIN_inter[5][0+1], C_out[5+1][0]);
		PE_array[5][1].compute<5,1>(A_inter[5][1], M_MIN_inter[5][1], C_out[5][1], A_inter[5][1+1], M_MIN_inter[5][1+1], C_out[5+1][1]);
		PE_array[5][2].compute<5,2>(A_inter[5][2], M_MIN_inter[5][2], C_out[5][2], A_inter[5][2+1], M_MIN_inter[5][2+1], C_out[5+1][2]);
		PE_array[5][3].compute<5,3>(A_inter[5][3], M_MIN_inter[5][3], C_out[5][3], A_inter[5][3+1], M_MIN_inter[5][3+1], C_out[5+1][3]);
		PE_array[5][4].compute<5,4>(A_inter[5][4], M_MIN_inter[5][4], C_out[5][4], A_inter[5][4+1], M_MIN_inter[5][4+1], C_out[5+1][4]);
		PE_array[5][5].compute<5,5>(A_inter[5][5], M_MIN_inter[5][5], C_out[5][5], A_inter[5][5+1], M_MIN_inter[5][5+1], C_out[5+1][5]);
		PE_array[5][6].compute<5,6>(A_inter[5][6], M_MIN_inter[5][6], C_out[5][6], A_inter[5][6+1], M_MIN_inter[5][6+1], C_out[5+1][6]);
		PE_array[5][7].compute<5,7>(A_inter[5][7], M_MIN_inter[5][7], C_out[5][7], A_inter[5][7+1], M_MIN_inter[5][7+1], C_out[5+1][7]);
		PE_array[5][8].compute<5,8>(A_inter[5][8], M_MIN_inter[5][8], C_out[5][8], A_inter[5][8+1], M_MIN_inter[5][8+1], C_out[5+1][8]);

		PE_array[6][0].compute<6,0>(     Ain_s[6],        M_MIN_s[6], C_out[6][0], A_inter[6][0+1], M_MIN_inter[6][0+1], Cout_s[0]);
		PE_array[6][1].compute<6,1>(A_inter[6][1], M_MIN_inter[6][1], C_out[6][1], A_inter[6][1+1], M_MIN_inter[6][1+1], Cout_s[1]);
		PE_array[6][2].compute<6,2>(A_inter[6][2], M_MIN_inter[6][2], C_out[6][2], A_inter[6][2+1], M_MIN_inter[6][2+1], Cout_s[2]);
		PE_array[6][3].compute<6,3>(A_inter[6][3], M_MIN_inter[6][3], C_out[6][3], A_inter[6][3+1], M_MIN_inter[6][3+1], Cout_s[3]);
		PE_array[6][4].compute<6,4>(A_inter[6][4], M_MIN_inter[6][4], C_out[6][4], A_inter[6][4+1], M_MIN_inter[6][4+1], Cout_s[4]);
		PE_array[6][5].compute<6,5>(A_inter[6][5], M_MIN_inter[6][5], C_out[6][5], A_inter[6][5+1], M_MIN_inter[6][5+1], Cout_s[5]);
		PE_array[6][6].compute<6,6>(A_inter[6][6], M_MIN_inter[6][6], C_out[6][6], A_inter[6][6+1], M_MIN_inter[6][6+1], Cout_s[6]);
		PE_array[6][7].compute<6,7>(A_inter[6][7], M_MIN_inter[6][7], C_out[6][7], A_inter[6][7+1], M_MIN_inter[6][7+1], Cout_s[7]);
		PE_array[6][8].compute<6,8>(A_inter[6][8], M_MIN_inter[6][8], C_out[6][8], A_inter[6][8+1], M_MIN_inter[6][8+1], Cout_s[8]);
	}

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
//////////////////////////////////////////v13-iws_c syn ok end////////////////////////////////////////

////////////////////////////////////////////v12-iws_c syn ok  start////////////////////////////////////////
//typedef struct {
//	bool wb;
//	bool init;
//} wb_wrap_s;
//
//class PE_cls {
//protected:
//	int weight_local;
//	int tmp_buf[TILE_M];
//public:
//template<int MIN_R,int MID_C>
//	void compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN);
//template<int MIN_R,int MID_C>
//	void fillB(hls::stream<int> &B_in, hls::stream<int> &B_out);
//};
//
//template<int MIN_R,int MID_C>
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN)
//{
//#pragma HLS INLINE off
//	for(int i = 0; i < M_MIN; i++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
//		int tmp, A_tmp;
//		A_in >> A_tmp;
//		tmp_buf[i] = A_tmp * weight_local;
//		A_out << A_tmp;
//	}
//
//	for(int i = 0; i < M_MIN; i++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
//		C_out << tmp_buf[i];
//	}
//
//	int trans_num = MIN_R*M_MIN;
//
//	for(int i = 0; i < trans_num; i++){
//#pragma HLS LOOP_TRIPCOUNT min=1 max=PE_TRANS_NUM_MAX
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=PE_TRANS_NUM_MAX)
//#pragma HLS PIPELINE II=1
//		int tmp;
//		C_in >> tmp;
//		C_out << tmp;
//	}
//
//	return;
//}
//
//template<int MIN_R,int MID_C>
//void PE_cls::fillB(hls::stream<int> &B_in, hls::stream<int> &B_out){
//#pragma HLS INLINE off
//	int B_tmp;
//	B_in >> B_tmp;
//	weight_local = B_tmp;
////tranfer neighbor PE's weight
//	for(int k=0; k<SA_R-1-MIN_R; k++){
//#pragma HLS PIPELINE II=1
//		int tmp_in, tmp_out;
//		B_in  >> tmp_in;
//		tmp_out = tmp_in;
//		B_out << tmp_out;
//	}
//}
//
//void Drain(hls::stream<int> &in, int data_num)
//{
//	int drain;
//	for(int k = 0; k<data_num; k++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS PIPELINE II=1
//		in >> drain;
//	}
//}
//
//void Writeback_SA_Cout(int C_local[TILE_M][SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb1_s)
//{
//	static int partial_sum[TILE_M][SA_C];
//#pragma HLS ARRAY_PARTITION variable=partial_sum complete dim=2
//	wb_wrap_s wbw_s;
//
//	wbw_s = wb1_s.read();
//	bool init = wbw_s.init;
//	bool wb = wbw_s.wb;
//
//	for(int k=0; k<SA_R;k++){
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
//		for(int i=0; i<M_MIN;i++)
//		{
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//#pragma HLS LOOP_TRIPCOUNT min=1 max=DEPTH_TILE_M
//#pragma HLS PIPELINE II=1
////#pragma HLS DEPENDENCE variable=partial_sum inter false
//			for(int j=0; j<SA_C; j++)
//			{
//				int tmp, psum;
//				if((k==0)&&init)
//					psum = 0;
//				else
//					psum = partial_sum[i][j];
//				tmp = Cout_s[j].read();
//				partial_sum[i][j] = psum + tmp;
//			}
//		}
//	}
//
//	if(wb){
//		for(int i=0; i<M_MIN; i++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//				for(int j=0; j<N_MIN; j++){
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//			C_local[i][j] = partial_sum[i][j];
//				}
//		}
//	}
//}
//
//void Compute_SA(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN, int N_MIN,
//		hls::stream<wb_wrap_s> &wb0_s, hls::stream<wb_wrap_s> &wb1_s)
//{
//	hls::stream<int> A_inter[SA_R][SA_C+1];
//#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_M
//#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_M
//	hls::stream<int> B_inter[SA_R+1][SA_C];
//#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_SA_R
//#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_SA_R
//	hls::stream<int> C_out[SA_R+1][SA_C];
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_TILE_M
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_TILE_M
//
//	static PE_cls PE_array[SA_R][SA_C];
//#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=1
//#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=2
//
////	for(int k=0; k<SA_R; k++){
////#pragma HLS UNROLL
////		for(int j=0; j<SA_C; j++){
////			int tmp_in, tmp_out;
////			tmp_in = Bin_s[j].read();
////			tmp_out = tmp_in;
////			B_inter[0][j] << tmp_out;
////		}
////	}
//
////	FILL_B:for(int i=0; i<SA_R; i++){
////#pragma HLS UNROLL
////		for(int j=0; j<SA_C; j++)
////		{
////#pragma HLS UNROLL
////			PE_array[i][j].fillB<0,0>(B_inter[i][j], B_inter[i+1][j], i);
////		}
////	}
//
////	FILL_B:{
////		for(int i=0; i<SA_R; i++)
////		{
////#pragma HLS UNROLL
////			PE_array[i][0].fillB<0,0>(B_inter[i][0], B_inter[i+1][0], i);
////			PE_array[i][1].fillB<0,1>(B_inter[i][1], B_inter[i+1][1], i);
////			PE_array[i][2].fillB<0,2>(B_inter[i][2], B_inter[i+1][2], i);
////			PE_array[i][3].fillB<0,3>(B_inter[i][3], B_inter[i+1][3], i);
////			PE_array[i][4].fillB<0,4>(B_inter[i][4], B_inter[i+1][4], i);
////			PE_array[i][5].fillB<0,5>(B_inter[i][5], B_inter[i+1][5], i);
////			PE_array[i][6].fillB<0,6>(B_inter[i][6], B_inter[i+1][6], i);
////			PE_array[i][7].fillB<0,7>(B_inter[i][7], B_inter[i+1][7], i);
////			PE_array[i][8].fillB<0,8>(B_inter[i][8], B_inter[i+1][8], i);
////		}
////	}
//
//	FILL_B:{
//		int i;
//		i=0;
////		PE_array[i][0].fillB<0,0>(B_inter[i][0], B_inter[i+1][0]);
////		PE_array[i][1].fillB<0,1>(B_inter[i][1], B_inter[i+1][1]);
////		PE_array[i][2].fillB<0,2>(B_inter[i][2], B_inter[i+1][2]);
////		PE_array[i][3].fillB<0,3>(B_inter[i][3], B_inter[i+1][3]);
////		PE_array[i][4].fillB<0,4>(B_inter[i][4], B_inter[i+1][4]);
////		PE_array[i][5].fillB<0,5>(B_inter[i][5], B_inter[i+1][5]);
////		PE_array[i][6].fillB<0,6>(B_inter[i][6], B_inter[i+1][6]);
////		PE_array[i][7].fillB<0,7>(B_inter[i][7], B_inter[i+1][7]);
////		PE_array[i][8].fillB<0,8>(B_inter[i][8], B_inter[i+1][8]);
//
//		PE_array[i][0].fillB<0,0>(Bin_s[0], B_inter[i+1][0]);
//		PE_array[i][1].fillB<0,1>(Bin_s[1], B_inter[i+1][1]);
//		PE_array[i][2].fillB<0,2>(Bin_s[2], B_inter[i+1][2]);
//		PE_array[i][3].fillB<0,3>(Bin_s[3], B_inter[i+1][3]);
//		PE_array[i][4].fillB<0,4>(Bin_s[4], B_inter[i+1][4]);
//		PE_array[i][5].fillB<0,5>(Bin_s[5], B_inter[i+1][5]);
//		PE_array[i][6].fillB<0,6>(Bin_s[6], B_inter[i+1][6]);
//		PE_array[i][7].fillB<0,7>(Bin_s[7], B_inter[i+1][7]);
//		PE_array[i][8].fillB<0,8>(Bin_s[8], B_inter[i+1][8]);
//		i=1;
//		PE_array[i][0].fillB<1,0>(B_inter[i][0], B_inter[i+1][0]);
//		PE_array[i][1].fillB<1,1>(B_inter[i][1], B_inter[i+1][1]);
//		PE_array[i][2].fillB<1,2>(B_inter[i][2], B_inter[i+1][2]);
//		PE_array[i][3].fillB<1,3>(B_inter[i][3], B_inter[i+1][3]);
//		PE_array[i][4].fillB<1,4>(B_inter[i][4], B_inter[i+1][4]);
//		PE_array[i][5].fillB<1,5>(B_inter[i][5], B_inter[i+1][5]);
//		PE_array[i][6].fillB<1,6>(B_inter[i][6], B_inter[i+1][6]);
//		PE_array[i][7].fillB<1,7>(B_inter[i][7], B_inter[i+1][7]);
//		PE_array[i][8].fillB<1,8>(B_inter[i][8], B_inter[i+1][8]);
//		i=2;
//		PE_array[i][0].fillB<2,0>(B_inter[i][0], B_inter[i+1][0]);
//		PE_array[i][1].fillB<2,1>(B_inter[i][1], B_inter[i+1][1]);
//		PE_array[i][2].fillB<2,2>(B_inter[i][2], B_inter[i+1][2]);
//		PE_array[i][3].fillB<2,3>(B_inter[i][3], B_inter[i+1][3]);
//		PE_array[i][4].fillB<2,4>(B_inter[i][4], B_inter[i+1][4]);
//		PE_array[i][5].fillB<2,5>(B_inter[i][5], B_inter[i+1][5]);
//		PE_array[i][6].fillB<2,6>(B_inter[i][6], B_inter[i+1][6]);
//		PE_array[i][7].fillB<2,7>(B_inter[i][7], B_inter[i+1][7]);
//		PE_array[i][8].fillB<2,8>(B_inter[i][8], B_inter[i+1][8]);
//		i=3;
//		PE_array[i][0].fillB<3,0>(B_inter[i][0], B_inter[i+1][0]);
//		PE_array[i][1].fillB<3,1>(B_inter[i][1], B_inter[i+1][1]);
//		PE_array[i][2].fillB<3,2>(B_inter[i][2], B_inter[i+1][2]);
//		PE_array[i][3].fillB<3,3>(B_inter[i][3], B_inter[i+1][3]);
//		PE_array[i][4].fillB<3,4>(B_inter[i][4], B_inter[i+1][4]);
//		PE_array[i][5].fillB<3,5>(B_inter[i][5], B_inter[i+1][5]);
//		PE_array[i][6].fillB<3,6>(B_inter[i][6], B_inter[i+1][6]);
//		PE_array[i][7].fillB<3,7>(B_inter[i][7], B_inter[i+1][7]);
//		PE_array[i][8].fillB<3,8>(B_inter[i][8], B_inter[i+1][8]);
//		i=4;
//		PE_array[i][0].fillB<4,0>(B_inter[i][0], B_inter[i+1][0]);
//		PE_array[i][1].fillB<4,1>(B_inter[i][1], B_inter[i+1][1]);
//		PE_array[i][2].fillB<4,2>(B_inter[i][2], B_inter[i+1][2]);
//		PE_array[i][3].fillB<4,3>(B_inter[i][3], B_inter[i+1][3]);
//		PE_array[i][4].fillB<4,4>(B_inter[i][4], B_inter[i+1][4]);
//		PE_array[i][5].fillB<4,5>(B_inter[i][5], B_inter[i+1][5]);
//		PE_array[i][6].fillB<4,6>(B_inter[i][6], B_inter[i+1][6]);
//		PE_array[i][7].fillB<4,7>(B_inter[i][7], B_inter[i+1][7]);
//		PE_array[i][8].fillB<4,8>(B_inter[i][8], B_inter[i+1][8]);
//		i=5;
//		PE_array[i][0].fillB<5,0>(B_inter[i][0], B_inter[i+1][0]);
//		PE_array[i][1].fillB<5,1>(B_inter[i][1], B_inter[i+1][1]);
//		PE_array[i][2].fillB<5,2>(B_inter[i][2], B_inter[i+1][2]);
//		PE_array[i][3].fillB<5,3>(B_inter[i][3], B_inter[i+1][3]);
//		PE_array[i][4].fillB<5,4>(B_inter[i][4], B_inter[i+1][4]);
//		PE_array[i][5].fillB<5,5>(B_inter[i][5], B_inter[i+1][5]);
//		PE_array[i][6].fillB<5,6>(B_inter[i][6], B_inter[i+1][6]);
//		PE_array[i][7].fillB<5,7>(B_inter[i][7], B_inter[i+1][7]);
//		PE_array[i][8].fillB<5,8>(B_inter[i][8], B_inter[i+1][8]);
//		i=6;
//		PE_array[i][0].fillB<6,0>(B_inter[i][0], B_inter[i+1][0]);
//		PE_array[i][1].fillB<6,1>(B_inter[i][1], B_inter[i+1][1]);
//		PE_array[i][2].fillB<6,2>(B_inter[i][2], B_inter[i+1][2]);
//		PE_array[i][3].fillB<6,3>(B_inter[i][3], B_inter[i+1][3]);
//		PE_array[i][4].fillB<6,4>(B_inter[i][4], B_inter[i+1][4]);
//		PE_array[i][5].fillB<6,5>(B_inter[i][5], B_inter[i+1][5]);
//		PE_array[i][6].fillB<6,6>(B_inter[i][6], B_inter[i+1][6]);
//		PE_array[i][7].fillB<6,7>(B_inter[i][7], B_inter[i+1][7]);
//		PE_array[i][8].fillB<6,8>(B_inter[i][8], B_inter[i+1][8]);
//	}
//
////	for(int k=0; k<M_MIN; k++){
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
////#pragma HLS PIPELINE II=1
////		for(int i=0; i<SA_R; i++){
////			int tmp_in, tmp_out;
////			tmp_in = Ain_s[i].read();
////			tmp_out = tmp_in;
////			A_inter[i][0] << tmp_out;
////		}
////	}
//
////	Loop_M:for(int i=0; i<SA_R; i++){
////#pragma HLS UNROLL
////		Loop_N:for(int j=0; j<SA_C; j++)
////		{
////#pragma HLS UNROLL
////			PE_array[i][j].compute<0,1>(A_inter[i][j], C_out[i][j], A_inter[i][j+1], C_out[i+1][j], M_MIN, i);
////		}
////	}
//
////	Loop_M:for(int i=0; i<SA_R; i++){
////#pragma HLS UNROLL
////		PE_array[i][0].compute<0,0>(A_inter[i][0], C_out[i][0], A_inter[i][0+1], C_out[i+1][0], M_MIN, i);
////		PE_array[i][1].compute<0,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], C_out[i+1][1], M_MIN, i);
////		PE_array[i][2].compute<0,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], C_out[i+1][2], M_MIN, i);
////		PE_array[i][3].compute<0,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], C_out[i+1][3], M_MIN, i);
////		PE_array[i][4].compute<0,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], C_out[i+1][4], M_MIN, i);
////		PE_array[i][5].compute<0,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], C_out[i+1][5], M_MIN, i);
////		PE_array[i][6].compute<0,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], C_out[i+1][6], M_MIN, i);
////		PE_array[i][7].compute<0,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], C_out[i+1][7], M_MIN, i);
////		PE_array[i][8].compute<0,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], C_out[i+1][8], M_MIN, i);
////	}
//
//	Compute_SA_Loop:{
//		int i;
//		i = 0;
//		PE_array[i][0].compute<0,0>(     Ain_s[i], C_out[i][0], A_inter[i][0+1], C_out[i+1][0], M_MIN);
//		PE_array[i][1].compute<0,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], C_out[i+1][1], M_MIN);
//		PE_array[i][2].compute<0,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], C_out[i+1][2], M_MIN);
//		PE_array[i][3].compute<0,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], C_out[i+1][3], M_MIN);
//		PE_array[i][4].compute<0,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], C_out[i+1][4], M_MIN);
//		PE_array[i][5].compute<0,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], C_out[i+1][5], M_MIN);
//		PE_array[i][6].compute<0,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], C_out[i+1][6], M_MIN);
//		PE_array[i][7].compute<0,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], C_out[i+1][7], M_MIN);
//		PE_array[i][8].compute<0,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], C_out[i+1][8], M_MIN);
//		i = 1;
//		PE_array[i][0].compute<1,0>(     Ain_s[i], C_out[i][0], A_inter[i][0+1], C_out[i+1][0], M_MIN);
//		PE_array[i][1].compute<1,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], C_out[i+1][1], M_MIN);
//		PE_array[i][2].compute<1,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], C_out[i+1][2], M_MIN);
//		PE_array[i][3].compute<1,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], C_out[i+1][3], M_MIN);
//		PE_array[i][4].compute<1,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], C_out[i+1][4], M_MIN);
//		PE_array[i][5].compute<1,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], C_out[i+1][5], M_MIN);
//		PE_array[i][6].compute<1,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], C_out[i+1][6], M_MIN);
//		PE_array[i][7].compute<1,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], C_out[i+1][7], M_MIN);
//		PE_array[i][8].compute<1,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], C_out[i+1][8], M_MIN);
//		i = 2;
//		PE_array[i][0].compute<2,0>(     Ain_s[i], C_out[i][0], A_inter[i][0+1], C_out[i+1][0], M_MIN);
//		PE_array[i][1].compute<2,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], C_out[i+1][1], M_MIN);
//		PE_array[i][2].compute<2,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], C_out[i+1][2], M_MIN);
//		PE_array[i][3].compute<2,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], C_out[i+1][3], M_MIN);
//		PE_array[i][4].compute<2,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], C_out[i+1][4], M_MIN);
//		PE_array[i][5].compute<2,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], C_out[i+1][5], M_MIN);
//		PE_array[i][6].compute<2,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], C_out[i+1][6], M_MIN);
//		PE_array[i][7].compute<2,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], C_out[i+1][7], M_MIN);
//		PE_array[i][8].compute<2,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], C_out[i+1][8], M_MIN);
//		i = 3;
//		PE_array[i][0].compute<3,0>(     Ain_s[i], C_out[i][0], A_inter[i][0+1], C_out[i+1][0], M_MIN);
//		PE_array[i][1].compute<3,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], C_out[i+1][1], M_MIN);
//		PE_array[i][2].compute<3,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], C_out[i+1][2], M_MIN);
//		PE_array[i][3].compute<3,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], C_out[i+1][3], M_MIN);
//		PE_array[i][4].compute<3,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], C_out[i+1][4], M_MIN);
//		PE_array[i][5].compute<3,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], C_out[i+1][5], M_MIN);
//		PE_array[i][6].compute<3,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], C_out[i+1][6], M_MIN);
//		PE_array[i][7].compute<3,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], C_out[i+1][7], M_MIN);
//		PE_array[i][8].compute<3,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], C_out[i+1][8], M_MIN);
//		i = 4;
//		PE_array[i][0].compute<4,0>(     Ain_s[i], C_out[i][0], A_inter[i][0+1], C_out[i+1][0], M_MIN);
//		PE_array[i][1].compute<4,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], C_out[i+1][1], M_MIN);
//		PE_array[i][2].compute<4,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], C_out[i+1][2], M_MIN);
//		PE_array[i][3].compute<4,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], C_out[i+1][3], M_MIN);
//		PE_array[i][4].compute<4,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], C_out[i+1][4], M_MIN);
//		PE_array[i][5].compute<4,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], C_out[i+1][5], M_MIN);
//		PE_array[i][6].compute<4,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], C_out[i+1][6], M_MIN);
//		PE_array[i][7].compute<4,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], C_out[i+1][7], M_MIN);
//		PE_array[i][8].compute<4,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], C_out[i+1][8], M_MIN);
//		i = 5;
//		PE_array[i][0].compute<5,0>(     Ain_s[i], C_out[i][0], A_inter[i][0+1], C_out[i+1][0], M_MIN);
//		PE_array[i][1].compute<5,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], C_out[i+1][1], M_MIN);
//		PE_array[i][2].compute<5,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], C_out[i+1][2], M_MIN);
//		PE_array[i][3].compute<5,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], C_out[i+1][3], M_MIN);
//		PE_array[i][4].compute<5,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], C_out[i+1][4], M_MIN);
//		PE_array[i][5].compute<5,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], C_out[i+1][5], M_MIN);
//		PE_array[i][6].compute<5,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], C_out[i+1][6], M_MIN);
//		PE_array[i][7].compute<5,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], C_out[i+1][7], M_MIN);
//		PE_array[i][8].compute<5,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], C_out[i+1][8], M_MIN);
//		i = 6;
//		PE_array[i][0].compute<6,0>(     Ain_s[i], C_out[i][0], A_inter[i][0+1], Cout_s[0], M_MIN);
//		PE_array[i][1].compute<6,1>(A_inter[i][1], C_out[i][1], A_inter[i][1+1], Cout_s[1], M_MIN);
//		PE_array[i][2].compute<6,2>(A_inter[i][2], C_out[i][2], A_inter[i][2+1], Cout_s[2], M_MIN);
//		PE_array[i][3].compute<6,3>(A_inter[i][3], C_out[i][3], A_inter[i][3+1], Cout_s[3], M_MIN);
//		PE_array[i][4].compute<6,4>(A_inter[i][4], C_out[i][4], A_inter[i][4+1], Cout_s[4], M_MIN);
//		PE_array[i][5].compute<6,5>(A_inter[i][5], C_out[i][5], A_inter[i][5+1], Cout_s[5], M_MIN);
//		PE_array[i][6].compute<6,6>(A_inter[i][6], C_out[i][6], A_inter[i][6+1], Cout_s[6], M_MIN);
//		PE_array[i][7].compute<6,7>(A_inter[i][7], C_out[i][7], A_inter[i][7+1], Cout_s[7], M_MIN);
//		PE_array[i][8].compute<6,8>(A_inter[i][8], C_out[i][8], A_inter[i][8+1], Cout_s[8], M_MIN);
//	}
//
//	DRAIN_A:for(int i=0; i<SA_R; i++)
//	{
//#pragma HLS UNROLL
//		Drain(A_inter[i][SA_C], M_MIN);
//	}
//
////	for(int i=0; i<SA_R*M_MIN;i++){
////#pragma HLS PIPELINE II=1
////		for(int j=0; j<SA_C; j++)
////		{
////			int tmp_in, tmp_out;
////			tmp_in = C_out[SA_R][j].read();
////			tmp_out = tmp_in;
////			Cout_s[j] << tmp_out;
////		}
////	}
//
//	wb_wrap_s wbw_s;
//	wbw_s = wb0_s.read();
//	wb1_s.write(wbw_s);
//}
//
//void Load_A(hls::stream<int> Ain_s[SA_R], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<SA_R; k++)
//			{
//#pragma HLS PIPELINE II=1
//				int tmp_in;
//				if(k<K_MIN)
//					tmp_in = A[base_offset + i*K_len + k];
//				else
//					tmp_in = 0;
//				Ain_s[k] << tmp_in;
//			}
//}
//
//void Load_B(hls::stream<int> Bin_s[SA_C], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<SA_R; i++)
//			Loop_N:for(int k=0; k<SA_C; k++)
//			{
//#pragma HLS PIPELINE II=1
//				int tmp_in;
//				bool k_en = i<K_MIN;
//				bool n_en = k<N_MIN;
//				bool ld_en = k_en && n_en;
//				if(ld_en)
//					tmp_in = B[base_offset + i*N_len + k];
//				else
//					tmp_in = 0;
//				Bin_s[k] << tmp_in;
//			}
//}
//
//void Store_C(int C_local[TILE_M][SA_C], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void Load_AB_wrapper(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], int *A, int *B, int i, int j, bool k_init,
//		int K, int N, int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb0_s){
//
//	static int k;
//	if(k_init){
//		k = 0;
//	}else{
//		k += SA_R;
//	}
//
//	int K_MIN = MIN(SA_R, K-k);
//
//	Load_A(Ain_s, A, i, k, K, M_MIN, K_MIN);
//	Load_B(Bin_s, B, k, j, N, K_MIN, N_MIN);
//
//	wb_wrap_s wbw_s0;
//	wbw_s0.wb = ((k+K_MIN) == K);
//	wbw_s0.init = (k == 0);
//	wb0_s.write(wbw_s0);
//}
//
//void DATAFLOW_SA_LCW(int C_local[TILE_M][SA_C], int *A, int *B, int M, int N, int K, int kloops,
//		int i, int j, int M_MIN, int N_MIN)
//{
//	hls::stream<int> Ain_s[SA_R];
//#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_M
//	hls::stream<int> Bin_s[SA_C];
//#pragma HLS STREAM variable=Bin_s depth=DEPTH_SA_R
//	hls::stream<int> Cout_s[SA_C];
//#pragma HLS STREAM variable=Cout_s depth=DEPTH_COUT_S
//
//	hls::stream<wb_wrap_s> wb0_s;
//	hls::stream<wb_wrap_s> wb1_s;
//
//	Loop_K:for(int k=0; k<kloops; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
//#pragma HLS DATAFLOW
//		Load_AB_wrapper( Ain_s, Bin_s, A, B, i, j, k==0, K, N, M_MIN, N_MIN, wb0_s);
//
//		Compute_SA(Ain_s, Bin_s, Cout_s, M_MIN, N_MIN, wb0_s, wb1_s);
//
//		Writeback_SA_Cout( C_local, Cout_s, M_MIN, N_MIN, wb1_s);
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
//	static int C_local[TILE_M][SA_C];
////#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN;
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_M)
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= SA_C)
//		{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_N)
//			N_MIN = MIN(SA_C, N-j);
//			DATAFLOW_SA_LCW( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v12-iws_c syn ok end////////////////////////////////////////

////////////////////////////////////////////v11-iws_c syn ok  start////////////////////////////////////////
//typedef struct {
//	bool wb;
//	bool init;
//} wb_wrap_s;
//
//class PE_cls {
//protected:
//	int weight_local;
//	int tmp_buf[TILE_M];
//public:
//	void compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN, int row_idx);
//	void fillB(hls::stream<int> &B_in, hls::stream<int> &B_out, int i);
//};
//
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN, int row_idx)
//{
//	for(int i = 0; i < M_MIN; i++){
//#pragma HLS PIPELINE II=1
//		int tmp, A_tmp;
//		A_in >> A_tmp;
//		tmp_buf[i] = A_tmp * weight_local;
//		A_out << A_tmp;
//	}
//
//	for(int i = 0; i < M_MIN; i++){
//#pragma HLS PIPELINE II=1
//		C_out << tmp_buf[i];
//	}
//
//	int trans_num = row_idx*M_MIN;
//
//	for(int i = 0; i < trans_num; i++){
//#pragma HLS PIPELINE II=1
//		int tmp;
//		C_in >> tmp;
//		C_out << tmp;
//	}
//
//	return;
//}
//
//void PE_cls::fillB(hls::stream<int> &B_in, hls::stream<int> &B_out, int i){
//	int B_tmp;
//	B_in >> B_tmp;
//	weight_local = B_tmp;
////tranfer neighbor PE's weight
//	for(int k=0; k<SA_R-1-i; k++){
//#pragma HLS PIPELINE II=1
//		int tmp_in, tmp_out;
//		B_in  >> tmp_in;
//		tmp_out = tmp_in;
//		B_out << tmp_out;
//	}
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
//void Writeback_SA_Cout(int C_local[TILE_M][SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb1_s)
//{
//	static int partial_sum[TILE_M][SA_C];
//#pragma HLS ARRAY_PARTITION variable=partial_sum complete dim=2
//	wb_wrap_s wbw_s;
//
//	wbw_s = wb1_s.read();
//	bool init = wbw_s.init;
//	bool wb = wbw_s.wb;
//
//	for(int k=0; k<SA_R;k++)
//		for(int i=0; i<M_MIN;i++)
//		{
//#pragma HLS PIPELINE II=1
//#pragma HLS DEPENDENCE variable=partial_sum inter false
//			for(int j=0; j<SA_C; j++)
//			{
//				int tmp, psum;
//				if((k==0)&&init)
//					psum = 0;
//				else
//					psum = partial_sum[i][j];
//				tmp = Cout_s[j].read();
//				partial_sum[i][j] = psum + tmp;
//			}
//		}
//
//	if(wb){
//		for(int i=0; i<M_MIN; i++)
//			for(int j=0; j<N_MIN; j++){
//#pragma HLS PIPELINE II=1
//				C_local[i][j] = partial_sum[i][j];
//			}
//	}
//}
//
//void Compute_SA(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN, int N_MIN,
//		hls::stream<wb_wrap_s> &wb0_s, hls::stream<wb_wrap_s> &wb1_s)
//{
//	hls::stream<int> A_inter[SA_R][SA_C+1];
//#pragma HLS STREAM variable=A_inter dim=1 depth=DEPTH_TILE_M
//#pragma HLS STREAM variable=A_inter dim=2 depth=DEPTH_TILE_M
//	hls::stream<int> B_inter[SA_R+1][SA_C];
//#pragma HLS STREAM variable=B_inter dim=1 depth=DEPTH_TILE_SA_R
//#pragma HLS STREAM variable=B_inter dim=2 depth=DEPTH_TILE_SA_R
//	hls::stream<int> C_out[SA_R+1][SA_C];
//#pragma HLS STREAM variable=C_out dim=1 depth=DEPTH_TILE_M
//#pragma HLS STREAM variable=C_out dim=2 depth=DEPTH_TILE_M
//
//	static PE_cls PE_array[SA_R][SA_C];
//#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=1
//#pragma HLS ARRAY_PARTITION variable=PE_array complete dim=2
//
//	for(int k=0; k<SA_R; k++){
//#pragma HLS UNROLL
//		for(int j=0; j<SA_C; j++){
//			int tmp_in, tmp_out;
//			tmp_in = Bin_s[j].read();
//			tmp_out = tmp_in;
//			B_inter[0][j] << tmp_out;
//		}
//	}
//
//	FILL_B:for(int i=0; i<SA_R; i++){
//#pragma HLS UNROLL
//		for(int j=0; j<SA_C; j++)
//		{
//#pragma HLS UNROLL
//			PE_array[i][j].fillB(B_inter[i][j], B_inter[i+1][j], i);
//		}
//	}
//
//	for(int k=0; k<M_MIN; k++){
//#pragma HLS PIPELINE II=1
//		for(int i=0; i<SA_R; i++){
//			int tmp_in, tmp_out;
//			tmp_in = Ain_s[i].read();
//			tmp_out = tmp_in;
//			A_inter[i][0] << tmp_out;
//		}
//	}
//
//	Loop_M:for(int i=0; i<SA_R; i++){
//#pragma HLS UNROLL
//		Loop_N:for(int j=0; j<SA_C; j++)
//		{
//#pragma HLS UNROLL
//			PE_array[i][j].compute(A_inter[i][j], C_out[i][j], A_inter[i][j+1], C_out[i+1][j], M_MIN, i);
//		}
//	}
//
//	DRAIN_A:for(int i=0; i<SA_R; i++)
//	{
//#pragma HLS UNROLL
//		Drain(A_inter[i][SA_C], M_MIN);
//	}
//
//	for(int i=0; i<SA_R*M_MIN;i++){
//#pragma HLS PIPELINE II=1
//		for(int j=0; j<SA_C; j++)
//		{
//			int tmp_in, tmp_out;
//			tmp_in = C_out[SA_R][j].read();
//			tmp_out = tmp_in;
//			Cout_s[j] << tmp_out;
//		}
//	}
//
//	wb_wrap_s wbw_s;
//	wbw_s = wb0_s.read();
//	wb1_s.write(wbw_s);
//}
//
//void Load_A(hls::stream<int> Ain_s[SA_R], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<SA_R; k++)
//			{
//#pragma HLS PIPELINE II=1
//				int tmp_in;
//				if(k<K_MIN)
//					tmp_in = A[base_offset + i*K_len + k];
//				else
//					tmp_in = 0;
//				Ain_s[k] << tmp_in;
//			}
//}
//
//void Load_B(hls::stream<int> Bin_s[SA_C], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<SA_R; i++)
//			Loop_N:for(int k=0; k<SA_C; k++)
//			{
//#pragma HLS PIPELINE II=1
//				int tmp_in;
//				bool k_en = i<K_MIN;
//				bool n_en = k<N_MIN;
//				bool ld_en = k_en && n_en;
//				if(ld_en)
//					tmp_in = B[base_offset + i*N_len + k];
//				else
//					tmp_in = 0;
//				Bin_s[k] << tmp_in;
//			}
//}
//
//void Store_C(int C_local[TILE_M][SA_C], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void Load_AB_wrapper(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], int *A, int *B, int i, int j, bool k_init,
//		int K, int N, int M_MIN, int N_MIN, hls::stream<wb_wrap_s> &wb0_s){
//
//	static int k;
//	if(k_init){
//		k = 0;
//	}else{
//		k += SA_R;
//	}
//
//	int K_MIN = MIN(SA_R, K-k);
//
//	Load_A(Ain_s, A, i, k, K, M_MIN, K_MIN);
//	Load_B(Bin_s, B, k, j, N, K_MIN, N_MIN);
//
//	wb_wrap_s wbw_s0;
//	wbw_s0.wb = ((k+K_MIN) == K);
//	wbw_s0.init = (k == 0);
//	wb0_s.write(wbw_s0);
//}
//
//void DATAFLOW_SA_LCW(int C_local[TILE_M][SA_C], int *A, int *B, int M, int N, int K, int kloops,
//		int i, int j, int M_MIN, int N_MIN)
//{
//	hls::stream<int> Ain_s[SA_R];
//#pragma HLS STREAM variable=Ain_s depth=DEPTH_TILE_M
//	hls::stream<int> Bin_s[SA_C];
//#pragma HLS STREAM variable=Bin_s depth=DEPTH_SA_R
//	hls::stream<int> Cout_s[SA_C];
//#pragma HLS STREAM variable=Cout_s depth=DEPTH_TILE_M
//
//	hls::stream<wb_wrap_s> wb0_s;
//	hls::stream<wb_wrap_s> wb1_s;
//
//	Loop_K:for(int k=0; k<kloops; k++)
//	{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=NUM_K)
//#pragma HLS DATAFLOW
//		Load_AB_wrapper( Ain_s, Bin_s, A, B, i, j, k==0, K, N, M_MIN, N_MIN, wb0_s);
//
//		Compute_SA(Ain_s, Bin_s, Cout_s, M_MIN, N_MIN, wb0_s, wb1_s);
//
//		Writeback_SA_Cout( C_local, Cout_s, M_MIN, N_MIN, wb1_s);
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
//	static int C_local[TILE_M][SA_C];
////#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN;
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= SA_C)
//		{
//			N_MIN = MIN(SA_C, N-j);
//			DATAFLOW_SA_LCW( C_local, A, B, M, N, K, kloops, i, j, M_MIN, N_MIN);
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v11-iws_c syn ok end////////////////////////////////////////

////////////////////////////////////////////v10-iws_c sim ok  start////////////////////////////////////////
//class PE_cls {
//protected:
//	int weight_local;
//	int tmp_buf[TILE_M];
//public:
//	void compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN, int row_idx);
//	void fillB(hls::stream<int> &B_in, hls::stream<int> &B_out, int i);
//};
//
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN, int row_idx)
//{
//	for(int i = 0; i < M_MIN; i++){
//		int tmp, A_tmp;
//		A_in >> A_tmp;
//		tmp_buf[i] = A_tmp * weight_local;
//		A_out << A_tmp;
//	}
//
//	for(int i = 0; i < M_MIN; i++){
//		C_out << tmp_buf[i];
//	}
//
//	int trans_num = row_idx*M_MIN;
//
//	for(int i = 0; i < trans_num; i++){
//		int tmp;
//		C_in >> tmp;
//		C_out << tmp;
//	}
//
//	return;
//}
//
//void PE_cls::fillB(hls::stream<int> &B_in, hls::stream<int> &B_out, int i){
//	int B_tmp;
//	B_in >> B_tmp;
//	weight_local = B_tmp;
////tranfer neighbor PE's weight
//	for(int k=0; k<SA_R-1-i; k++){
//#pragma HLS PIPELINE II=1
//		int tmp_in, tmp_out;
//		B_in  >> tmp_in;
//		tmp_out = tmp_in;
//		B_out << tmp_out;
//	}
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
//void Writeback_SA_Cout(int C_local[TILE_M][SA_C], hls::stream<int> Cout_s[SA_C], bool init, int M_MIN, int N_MIN, bool wb)
//{
//
//	static int partial_sum[TILE_M][SA_C];
//
//	if(init){
//		for(int j=0; j<SA_C; j++)
//			for(int i=0; i<M_MIN;i++)
//			{
//					partial_sum[i][j] = 0;
//			}
//	}
//
//	for(int j=0; j<SA_C; j++)
//		for(int k=0; k<SA_R;k++)
//			for(int i=0; i<M_MIN;i++)
//			{
//				int tmp;
//				tmp = Cout_s[j].read();
//				partial_sum[i][j] += tmp;
//			}
//
//	if(wb){
//		for(int i=0; i<M_MIN; i++)
//			for(int j=0; j<N_MIN; j++){
//				C_local[i][j] = partial_sum[i][j];
//			}
//	}
//}
//
//void Compute_SA(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], hls::stream<int> Cout_s[SA_C], int M_MIN, int N_MIN)
//{
//	hls::stream<int> A_inter[SA_R][SA_C+1];
//#pragma HLS STREAM variable=A_inter
//	hls::stream<int> B_inter[SA_R+1][SA_C];
//#pragma HLS STREAM variable=B_inter
//	hls::stream<int> C_out[SA_R+1][SA_C];
//#pragma HLS STREAM variable=C_out
//
//	static PE_cls PE_array[SA_R][SA_C];
//
//	for(int k=0; k<SA_R; k++)
//		for(int j=0; j<SA_C; j++){
//			int tmp_in, tmp_out;
//			tmp_in = Bin_s[j].read();
//			tmp_out = tmp_in;
//			B_inter[0][j] << tmp_out;
//		}
//
//	FILL_B:for(int i=0; i<SA_R; i++){
//		for(int j=0; j<SA_C; j++)
//		{
//			PE_array[i][j].fillB(B_inter[i][j], B_inter[i+1][j], i);
//		}
//	}
//
//	for(int k=0; k<M_MIN; k++)
//		for(int i=0; i<SA_R; i++){
//			int tmp_in, tmp_out;
//			tmp_in = Ain_s[i].read();
//			tmp_out = tmp_in;
//			A_inter[i][0] << tmp_out;
//		}
//
//	Loop_M:for(int i=0; i<SA_R; i++){
//		Loop_N:for(int j=0; j<SA_C; j++)
//		{
//			PE_array[i][j].compute(A_inter[i][j], C_out[i][j], A_inter[i][j+1], C_out[i+1][j], M_MIN, i);
//		}
//	}
//
//	for(int i=0; i<SA_R; i++)
//	{
//		Drain(A_inter[i][SA_C], M_MIN);
//	}
//
//	for(int j=0; j<SA_C; j++)
//		for(int i=0; i<SA_R*M_MIN;i++)
//		{
//			int tmp_in, tmp_out;
//			tmp_in = C_out[SA_R][j].read();
//			tmp_out = tmp_in;
//			Cout_s[j] << tmp_out;
//		}
//}
//
//void Load_A(int A_local[TILE_M][SA_R], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<SA_R; k++)
//			{
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
//#pragma HLS PIPELINE II=1
//				int tmp;
//				if(k<K_MIN)
//					tmp = A[base_offset + i*K_len + k];
//				else
//					tmp = 0;
//				A_local[i][k] = tmp;
//			}
//}
//
//void Load_B(int B_local[SA_R][SA_C], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<SA_R; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//				int tmp;
//				if(i<K_MIN)
//					tmp = B[base_offset + i*N_len + k];
//				else
//					tmp = 0;
//				B_local[i][k] = tmp;
//			}
//}
//
//void Store_C(int C_local[TILE_M][SA_C], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
//}
//
//void Load_AB_wrapper(hls::stream<int> Ain_s[SA_R], hls::stream<int> Bin_s[SA_C], int *A, int *B, int i, int j, bool k_init,
//		int K, int N, int M_MIN, int N_MIN, bool wb0[1], bool init0[1]){
//
//	static int A_local[TILE_M][SA_R];
//#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
//	static int B_local[SA_R][SA_C];
//#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2
//
//	static int k;
//	if(k_init){
//		k = 0;
//	}else{
//		k += SA_R;
//	}
//
//	int K_MIN = MIN(SA_R, K-k);
//
//	Load_A(A_local, A, i, k, K, M_MIN, K_MIN);
//	Load_B(B_local, B, k, j, N, K_MIN, N_MIN);
//
//	for(int k=0; k<SA_R; k++)
//		for(int j=0; j<SA_C; j++){
//			int tmp;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			else
//				tmp = 0;
//			Bin_s[j] << tmp;
//		}
//
//	for(int k=0; k<M_MIN; k++)
//		for(int i=0; i<SA_R; i++){
//			Ain_s[i] << A_local[k][i];
//		}
//
//	wb0[0] = ((k+K_MIN) == K);
//	init0[0] = (k == 0);
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
//
//#pragma HLS INTERFACE s_axilite register port=A bundle=CB
//#pragma HLS INTERFACE s_axilite register port=B bundle=CB
//#pragma HLS INTERFACE s_axilite register port=C bundle=CB
//
//	static int C_local[TILE_M][SA_C];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	hls::stream<int> Ain_s[SA_R];
//#pragma HLS STREAM variable=Ain_s
//	hls::stream<int> Bin_s[SA_C];
//#pragma HLS STREAM variable=Bin_s
//	hls::stream<int> Cout_s[SA_C];
//#pragma HLS STREAM variable=Cout_s
//
//	int M_MIN, N_MIN;
//	bool wb0[1], init0[1];
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= SA_C)
//		{
//			N_MIN = MIN(SA_C, N-j);
//			Loop_K:for(int k=0; k<kloops; k++)
//			{
//				Load_AB_wrapper( Ain_s, Bin_s, A, B, i, j, k==0, K, N, M_MIN, N_MIN, wb0, init0);
//
//				Compute_SA(Ain_s, Bin_s, Cout_s, M_MIN, N_MIN);
//
//				Writeback_SA_Cout( C_local, Cout_s, init0[0], M_MIN, N_MIN, wb0[0]);
//			}
//
//			Store_C(C_local, C, i, j, N, M_MIN, N_MIN);
//		}
//	}
//}
////////////////////////////////////////////v10-iws_c sim ok end////////////////////////////////////////

////////////////////////////////////////////v9-iws_c sim ok  start////////////////////////////////////////
//
//#define SA_R TILE_M
//#define SA_C TILE_N
//
//class PE_cls {
//protected:
//	int weight_local;
//	int tmp_buf[TILE_M];
//public:
//	void compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN, int row_idx);
//	void fillB(hls::stream<int> &B_in, hls::stream<int> &B_out, int i);
//};
//
//void PE_cls::compute(hls::stream<int> &A_in, hls::stream<int> &C_in, hls::stream<int> &A_out, hls::stream<int> &C_out, int M_MIN, int row_idx)
//{
//	for(int i = 0; i < M_MIN; i++){
//		int tmp, A_tmp;
//		A_in >> A_tmp;
//		tmp_buf[i] = A_tmp * weight_local;
//		A_out << A_tmp;
//	}
//
//	for(int i = 0; i < M_MIN; i++){
//		C_out << tmp_buf[i];
//	}
//
//	int trans_num = row_idx*M_MIN;
//
//	for(int i = 0; i < trans_num; i++){
//		int tmp;
//		C_in >> tmp;
//		C_out << tmp;
//	}
//
//	return;
//}
//
//void PE_cls::fillB(hls::stream<int> &B_in, hls::stream<int> &B_out, int i){
//	int B_tmp;
//	B_in >> B_tmp;
//	weight_local = B_tmp;
////tranfer neighbor PE's weight
//	for(int k=0; k<SA_R-1-i; k++){
//#pragma HLS PIPELINE II=1
//		int tmp_in, tmp_out;
//		B_in  >> tmp_in;
//		tmp_out = tmp_in;
//		B_out << tmp_out;
//	}
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
//void Compute_SA(int A_local[TILE_M][SA_R], int B_local[SA_R][SA_C], int C_local[TILE_M][SA_C], bool init, int M_MIN, int N_MIN, int K_MIN, int K_MAX)
//{
//	hls::stream<int> A_inter[SA_R][SA_C+1];
//#pragma HLS STREAM variable=A_inter
//	hls::stream<int> B_inter[SA_R+1][SA_C];
//#pragma HLS STREAM variable=B_inter
//	hls::stream<int> C_out[SA_R+1][SA_C];
//#pragma HLS STREAM variable=C_out
//
//	static PE_cls PE_array[SA_R][SA_C];
//	static int K_cnt;
//
//	for(int k=0; k<SA_R; k++)
//		for(int j=0; j<SA_C; j++){
//			int tmp;
//			if(j<N_MIN)
//				tmp = B_local[k][j];
//			else
//				tmp = 0;
//			B_inter[0][j] << tmp;
//		}
//
//	FILL_B:for(int i=0; i<SA_R; i++){
//		for(int j=0; j<SA_C; j++)
//		{
//			PE_array[i][j].fillB(B_inter[i][j], B_inter[i+1][j], i);
//		}
//	}
//
//	for(int k=0; k<M_MIN; k++)
//		for(int i=0; i<SA_R; i++){
//			A_inter[i][0] << A_local[k][i];
//		}
//
//	Loop_M:for(int i=0; i<SA_R; i++){
//		Loop_N:for(int j=0; j<SA_C; j++)
//		{
//			PE_array[i][j].compute(A_inter[i][j], C_out[i][j], A_inter[i][j+1], C_out[i+1][j], M_MIN, i);
//		}
//	}
//
//	for(int i=0; i<SA_R; i++)
//	{
//		Drain(A_inter[i][SA_C], M_MIN);
//	}
//
//	static int partial_sum[TILE_M][SA_C];
//
//	for(int j=0; j<SA_C; j++)
//		for(int i=0; i<M_MIN;i++)
//		{
//			if(init)
//				partial_sum[i][j] = 0;
//		}
//
//	for(int j=0; j<SA_C; j++)
//		for(int k=0; k<SA_R;k++)
//		{
//			for(int i=0; i<M_MIN;i++)
//			{
//				int tmp;
//				tmp = C_out[SA_R][j].read();
//				partial_sum[i][j] += tmp;
//			}
//		}
//
//
//	if(init){
//		K_cnt = 0;
//	}
//	K_cnt += K_MIN;
//
//	if(K_cnt == K_MAX){
//		for(int i=0; i<M_MIN; i++)
//			for(int j=0; j<N_MIN; j++){
//				C_local[i][j] = partial_sum[i][j];
//			}
//	}
//}
//
//void Load_A(int A_local[TILE_M][SA_R], int *A, int M_base, int K_base, int K_len, int M_MIN, int K_MIN)
//{
//	int base_offset = M_base*K_len + K_base;
//	Loop_M:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_K:for(int k=0; k<SA_R; k++)
//			{
////DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
//#pragma HLS PIPELINE II=1
//				int tmp;
//				if(k<K_MIN)
//					tmp = A[base_offset + i*K_len + k];
//				else
//					tmp = 0;
//				A_local[i][k] = tmp;
//			}
//}
//
//void Load_B(int B_local[SA_R][SA_C], int *B, int K_base, int N_base, int N_len, int K_MIN, int N_MIN)
//{
//	int base_offset = K_base*N_len + N_base;
//	Loop_K:for(int i=0; i<SA_R; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_R)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//				int tmp;
//				if(i<K_MIN)
//					tmp = B[base_offset + i*N_len + k];
//				else
//					tmp = 0;
//				B_local[i][k] = tmp;
//			}
//}
//
//void Store_C(int C_local[TILE_M][SA_C], int *C, int M_base, int N_base, int N_len, int M_MIN, int N_MIN)
//{
//	int base_offset = M_base*N_len + N_base;
//	Loop_K:for(int i=0; i<M_MIN; i++)
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=TILE_M)
//			Loop_N:for(int k=0; k<N_MIN; k++)
//			{
//DO_PRAGMA(HLS LOOP_TRIPCOUNT min=1 max=SA_C)
//#pragma HLS PIPELINE II=1
//				C[base_offset + i*N_len + k] = C_local[i][k];
//			}
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
//
//#pragma HLS INTERFACE s_axilite register port=A bundle=CB
//#pragma HLS INTERFACE s_axilite register port=B bundle=CB
//#pragma HLS INTERFACE s_axilite register port=C bundle=CB
//
//	static int A_local[TILE_M][SA_R];
//#pragma HLS ARRAY_PARTITION variable=A_local complete dim=1
//	static int B_local[SA_R][SA_C];
//#pragma HLS ARRAY_PARTITION variable=B_local complete dim=2
//	static int C_local[TILE_M][SA_C];
//#pragma HLS ARRAY_PARTITION variable=C_local complete dim=1
//
//	int M_MIN, N_MIN, K_MIN;
//
//	Loop_M:for(int i=0; i<M; i+= TILE_M)
//	{
//		M_MIN = MIN(TILE_M, M-i);
//		Loop_N:for(int j=0; j<N; j+= SA_C)
//		{
//			N_MIN = MIN(SA_C, N-j);
//			Loop_K:for(int k=0; k<K; k+= SA_R)
//			{
//				K_MIN = MIN(SA_R, K-k);
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
////////////////////////////////////////////v9-iws_c sim ok end////////////////////////////////////////

////////////////////////////////////////////v4 start////////////////////////////////////////
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
//void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops)//A[MxK]*B[KxN]=C[MxN]
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

//////////////////////////////////////////v1 start////////////////////////////////////////
//void MUL(int *A, int *B, int *C, int M, int N, int K, int kloops)//A[MxK]*B[KxN]=C[MxN]
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


