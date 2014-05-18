/*
 * Description: FPGA
 */
#include <soc/soc.h>
#include <reg.h>

static void writefpga(int c){

//	volatile char *base = (char *)versatile_fpga_vbase;	
 
//	 *(base) = c;
}

static int readfpga(void)	{

//	volatile char *base = (char *)versatile_fpga_vbase;		

	int c = 0;
 
//	c = *(base);

 	return c;
}	

void soc_writefpga(int c){
	writefpga(c);
}

int soc_readfpga(void){
	return readfpga();
}

