
#include <stdio.h>
#include <stdlib.h>
#include "systolic_array.h"

static int Array_A[NUM_M*NUM_K];
static int Array_B[NUM_K*NUM_N];
static int Array_C[NUM_M*NUM_N];
//static int Array_C_Golden[NUM_M*NUM_N];

int main(int argc, char *argv[]){

	FILE *out_file;
	out_file = fopen("output.dat","wb");

	printf("TEST Systolic Array\n");
	printf("=========START==========\n");

//	for(int i=0; i<NUM_M; i++)
//		for(int j=0; j<NUM_K; j++)
//		{
//			Array_A[i*NUM_K + j] = rand()%100;
//		}
//
//	for(int i=0; i<NUM_K; i++)
//		for(int j=0; j<NUM_N; j++)
//		{
//			Array_B[i*NUM_N + j] = rand()%100;
//		}

	for(int i=0; i<NUM_M; i++)
		for(int j=0; j<NUM_K; j++)
		{
			Array_A[i*NUM_K + j] = (i*3+j*7)%100;
		}

	for(int i=0; i<NUM_K; i++)
		for(int j=0; j<NUM_N; j++)
		{
			Array_B[i*NUM_N + j] = (i*5+j*13)%100;
		}

//	printf("sizeof(Array_C_Golden)=%d\n", sizeof(Array_C_Golden));
//	memset(Array_C_Golden, 0, sizeof(Array_C_Golden));
//
//	for(int i=0; i<NUM_M; i++)
//		for(int j=0; j<NUM_N; j++)
//			for(int k=0; k<NUM_K; k++)
//			{
//				Array_C_Golden[i*NUM_N + j] += Array_A[i*NUM_K + k]*Array_B[k*NUM_N + j];
//			}


	MUL( Array_A, Array_B, Array_C, NUM_M, NUM_N, NUM_K);

//	int err = 0;
//	for(int i=0; i<NUM_M*NUM_N; i++){
//		if((Array_C_Golden[i]-Array_C[i])!=0){
//			printf("[%d]:%d!=%d\n", i, Array_C[i], Array_C_Golden[i]);
//			err++;
//		}
//	}

	fwrite(Array_C,sizeof(int),NUM_M*NUM_N,out_file);
	fclose(out_file);

	int err = 0;
	err = system("diff --brief -w output.dat output.golden.dat");

	printf("err_num = %d\n", err);
	printf("==========END===========\n");
	return err;
}
