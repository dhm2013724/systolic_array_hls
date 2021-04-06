
#include "cnn.h"

double what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void copy_mem2dev(uint8_t *orig,uint32_t byte_num, unsigned long in_buffer)
{
	int fd = open("/dev/mem", O_RDWR);
	unsigned char *virtual_addr;
	uint32_t RequestByteNum;// must page
	if(byte_num%(HPAGESIZE)==0)
		RequestByteNum = byte_num;
	else
	{
		RequestByteNum = ceil(byte_num/(HPAGESIZE*1.0))*(HPAGESIZE);
	}
	virtual_addr = (unsigned char *)mmap(NULL, RequestByteNum, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)in_buffer);
	if(virtual_addr == MAP_FAILED)
	{
		perror("Virtual_addr_in mappong for absolute memory access failed!\n");
		return;
	}
	memcpy(virtual_addr,orig,byte_num);

	munmap((void *)virtual_addr, byte_num);
	close(fd);
}

void copy_dev2mem(uint8_t *dst,uint32_t byte_num, unsigned long in_buffer)
{
	int fd = open("/dev/mem", O_RDWR);
	unsigned char *virtual_addr;
	uint32_t RequestByteNum;// must page
	if(byte_num%(HPAGESIZE)==0)
		RequestByteNum = byte_num;
	else
	{
		RequestByteNum = ceil(byte_num/(HPAGESIZE*1.0))*(HPAGESIZE);
	}
		virtual_addr = (unsigned char *)mmap(NULL, RequestByteNum, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)in_buffer);
	if(virtual_addr == MAP_FAILED)
	{
		perror("Virtual_addr_in mappong for absolute memory access failed!\n");
		return;
	}
	memcpy((uint8_t *)dst,virtual_addr,byte_num);

	munmap((void *)virtual_addr, byte_num);
	close(fd);
}

int copy_file2mem(char *bin_file,uint32_t byte_num,unsigned long in_buffer)
{
	unsigned char *buffer = (unsigned char *)malloc(HPAGESIZE);
	if(buffer==NULL){
		printf("cannot malloc buffer %d byte\n", HPAGESIZE);
		return -1;
	}
	printf("Total Byte Num = %d\n Address 0x%X\n", byte_num, in_buffer);
	FILE *fp;
	if( (fp = fopen(bin_file, "rb")) == NULL)fprintf(stderr,"CANNOT OPEN bin_file\n");
	int rd_num;
	unsigned long offset = 0;
	while(rd_num = fread(buffer, sizeof(unsigned char), HPAGESIZE, fp))
	{
		if(rd_num < HPAGESIZE)
			rd_num = HPAGESIZE;
		copy_mem2dev(buffer,rd_num, in_buffer+offset);
		printf("rd_num=%d, offset=%d\n", rd_num, offset);
		offset += rd_num;
	}
	printf("copy_file2mem offset=%d\n",offset);
	fclose(fp);

	free(buffer);


	return 0;
}

int copy_mem2file(char *bin_file,uint32_t byte_num,unsigned long in_buffer)
{
	void *buffer = malloc(HPAGESIZE);
	if(buffer==NULL){
		printf("cannot malloc buffer %d byte\n", HPAGESIZE);
		return -1;
	}

	FILE *fp;
	if( (fp = fopen(bin_file, "wb")) == NULL)fprintf(stderr,"CANNOT OPEN bin_file\n");

	int x = byte_num;
	int addbyte;
	unsigned long offset = 0;
	while(addbyte=((x<HPAGESIZE)?x:(HPAGESIZE)))
	{
		copy_dev2mem((uint8_t *)buffer,addbyte, in_buffer+offset);
		fwrite(buffer , sizeof(unsigned char), addbyte, fp);
		x -= addbyte;
		offset += addbyte;
	}
	printf("copy_mem2file offset=%d\n",offset);


	fclose(fp);

	free(buffer);

	return 0;
}

int MUL(int *A, int *B, int *C, int M, int N, int K)//A[MxK]*B[KxN]=C[MxN]
{

	copy_mem2dev((uint8_t *)A, M*K*4, AIN_BASEADDR);
	copy_mem2dev((uint8_t *)B, K*N*4, BIN_BASEADDR);

	unsigned int ap_idle;
	unsigned int ap_done;

	unsigned long int PhysicalAddress = ACC_BASEADDR;
	int map_len = 0x80;
	int fd = open("/dev/mem", O_RDWR);

	unsigned char *xbase_address;
	xbase_address = (unsigned char *)mmap(NULL, map_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)PhysicalAddress);
	if(xbase_address == MAP_FAILED)
	{
		perror("1:Init Mapping memory for absolute memory access failed.\n");
		return -1;
	}

	while(1)
	{
		ap_idle = ((ReadReg(xbase_address, XMUL_CB_ADDR_AP_CTRL) >> 2) && 0x1);
		if(ap_idle)
			break;
	}

	WriteReg(xbase_address, XMUL_CB_ADDR_A_DATA,  AIN_BASEADDR);
	WriteReg(xbase_address, XMUL_CB_ADDR_B_DATA,  BIN_BASEADDR);
	WriteReg(xbase_address, XMUL_CB_ADDR_C_DATA,  COUT_BASEADDR);
	WriteReg(xbase_address, XMUL_CB_ADDR_M_DATA,  M);
	WriteReg(xbase_address, XMUL_CB_ADDR_N_DATA,  N);
	WriteReg(xbase_address, XMUL_CB_ADDR_K_DATA,  K);

	WriteReg(xbase_address, XMUL_CB_ADDR_GIE, 0x0);
	WriteReg(xbase_address, XMUL_CB_ADDR_AP_CTRL, 0x1);//Start
	while(1)
	{
		ap_done = ((ReadReg(xbase_address, XMUL_CB_ADDR_AP_CTRL) >> 1) && 0x1);
		if(ap_done)
			break;
	}

	munmap((void *)xbase_address, map_len);
	close(fd);

	copy_dev2mem((uint8_t *)C, M*N*4, COUT_BASEADDR);

	return 0;
}
