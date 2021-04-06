#ifndef __MNV1_H
#define __MNV1_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include "xfpga_acc_hw.h"

double what_time_is_it_now();

#define NUM_M 32
#define NUM_N 64
#define NUM_K 12

#define ACC_BASEADDR     0x00A0000000

#define AIN_BASEADDR    0x60000000//
#define BIN_BASEADDR    0x61000000//
#define COUT_BASEADDR   0x62000000//

#define WriteReg(BaseAddress, RegOffset, Data) *(volatile unsigned int*)((BaseAddress) + (RegOffset)) = (Data)
#define ReadReg(BaseAddress, RegOffset) *(volatile unsigned int*)((BaseAddress) + (RegOffset))

#define MIN_diy(x,y) ((x) < (y) ? (x) : (y))
#define MAX_diy(x,y) ((x) > (y) ? (x) : (y))

#define HPAGESIZE (2*1024*1024)

void copy_mem2dev(uint8_t *orig,uint32_t byte_num, unsigned long in_buffer);

void copy_dev2mem(uint8_t *dst,uint32_t byte_num, unsigned long in_buffer);

int copy_file2mem(char *bin_file,uint32_t byte_num,unsigned long in_buffer);

int copy_mem2file(char *bin_file,uint32_t byte_num,unsigned long in_buffer);

int MUL(int *A, int *B, int *C, int M, int N, int K);//A[MxK]*B[KxN]=C[MxN]

#endif
