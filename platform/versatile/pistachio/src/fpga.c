/*
 * Description: FPGA
 */
#include <soc/soc.h>
#include <reg.h>

static void writefpga(word_t c){

//	volatile char *base = (char *)versatile_fpga_vbase;	
 
//	 *(base) = c;
}

static word_t readfpga(void)	{

//	volatile char *base = (char *)versatile_fpga_vbase;		

	word_t c = 0;
 
//	c = *(base);

 	return c;
}	

void soc_writefpga(word_t c){
	writefpga(c);
}

word_t soc_readfpga(void){
	return readfpga();
}

