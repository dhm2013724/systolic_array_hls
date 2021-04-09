
#include "cnn.h"

//#define NUM_M 32
//#define NUM_N 64
//#define NUM_K 12

int main(int argc, char *argv[]){

	int NUM_M = 32;
	int NUM_N = 64;
	int NUM_K = 12;

	if(argc>1)
	{
		NUM_M = atoi(argv[1]);
		NUM_K = atoi(argv[2]);
		NUM_N = atoi(argv[3]);
	}

	printf("M=%d, K=%d, N=%d\n", NUM_M, NUM_K, NUM_N);
	int *Array_A        = (int *)malloc(NUM_M*NUM_K*sizeof(int));
	int *Array_B        = (int *)malloc(NUM_K*NUM_N*sizeof(int));
	int *Array_C        = (int *)malloc(NUM_M*NUM_N*sizeof(int));
	int *Array_C_Golden = (int *)malloc(NUM_M*NUM_N*sizeof(int));
	if((!Array_A)||(!Array_B)||(!Array_C)||(!Array_C_Golden)){
		printf("alloc failed.\n");
		return -1;
	}

	printf("TEST Systolic Array\n");
	printf("=========START==========\n");

	for(int i=0; i<NUM_M; i++)
		for(int j=0; j<NUM_K; j++)
		{
			Array_A[i*NUM_K + j] = rand()%100;
		}

	for(int i=0; i<NUM_K; i++)
		for(int j=0; j<NUM_N; j++)
		{
			Array_B[i*NUM_N + j] = rand()%100;
		}

	memset(Array_C_Golden, 0, NUM_M*NUM_N*sizeof(int));

	double cpu_time, fpga_time;
	double start_t, end_t;

	start_t = what_time_is_it_now();
	for(int i=0; i<NUM_M; i++)
		for(int j=0; j<NUM_N; j++)
			for(int k=0; k<NUM_K; k++)
			{
				Array_C_Golden[i*NUM_N + j] += Array_A[i*NUM_K + k]*Array_B[k*NUM_N + j];
			}
	end_t = what_time_is_it_now();
	cpu_time = end_t - start_t;
	printf(" CPU Total time:%3.7lf\n", cpu_time);

	start_t = what_time_is_it_now();
	MUL( Array_A, Array_B, Array_C, NUM_M, NUM_N, NUM_K);
	end_t = what_time_is_it_now();
	fpga_time = end_t - start_t;
	printf("FPGA Total time:%3.7lf\n", fpga_time);
	printf("FPGA/CPU speed = %3.7lf\n", cpu_time/fpga_time);

	int err = 0;
	for(int i=0; i<NUM_M*NUM_N; i++){
		if((Array_C_Golden[i]-Array_C[i])!=0){
			printf("[%d]:%d!=%d\n", i, Array_C[i], Array_C_Golden[i]);
			err++;
		}
	}
	printf("err_num = %d\n", err);

	free(Array_A);
	free(Array_B);
	free(Array_C);
	free(Array_C_Golden);

	printf("==========END===========\n");
	return err;
}
