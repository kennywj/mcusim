/* states */
#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "cmd.h"

#define POOL_KER_DIM 2
#define POOL_STRIDE 2
#define POOL_PADDING 0

q7_t scratch_buffer[1024 * 1600] __attribute__ ((aligned(64*(0x1<<10))));

/*
*
*	input feature map: width (w1) high (h1) channel (c1), 
*	parameters kernel number (n), kernel size (k), sride (s), padding (p)
*	output feature map: width (w2) high (h2) channel (c2)
*		w2 = (w1 - k + 2 * p)/s + 1
*		h2 = (h1 - k + 2 * p)/s + 1
*		c2 = n
*	weight number = k * k * c1 * n
*	bias number = n
*
* for example:
*	 input 5x5x3 feature map. channel 1
*    kernel number 1, kernel size 3x3, stride 1, passing 0
*	 the output feature map
*	 w2 = (5-3+2*0)/1 + 1 = 3
*    h2 = (5-3+2*0)/1 + 1 = 3
*    c2 = 1
*    so, the output feature map is 3x3x1
*    the weight number 3x3x1x1 = 9
*    the bias = 1
*/
// it need inital buffer before test because the ARM enable DSP (ARM_MATH_DSP in arm_math.h) 
// and the original data buffer will be reused.
#define BUFINIT(buf, size)	do {	\
	for (int i=0; i<size; i++)		\
		buf[i] = (q7_t)i & 0x7f;	\
	} while(0)

/**
  * @brief  do pool test and calculate sha1 digest
  * @param  argc
  * @param  argv
  * @retval None
  */
void cmd_pool(int argc, char* argv[])
{
	unsigned char digest[20];
	int i, in_ch, in_dim, pool_out_dim;
	
	memset(scratch_buffer, 0, sizeof(q7_t)*sizeof(scratch_buffer));
    q7_t *img_buffer1 = scratch_buffer;
    q7_t *img_buffer2 = &scratch_buffer[1024*800];
	
	printf("ARM maxpool test\n");
	// it need inital buffer before test because the ARM enable DSP and the original data buffer will be reused.
	BUFINIT(img_buffer1, 1024*800);
	sha1sum("origin",img_buffer1, 1024*800, digest);
	
	 // test case 1
    in_ch = 16;
    in_dim = 22;
    pool_out_dim = 11;
	printf("case 1: input channel %d, input dimension %d, output dimension %d\n",in_ch, in_dim, pool_out_dim);
	
	arm_maxpool_q7_HWC(	img_buffer1, in_dim, in_ch, 
						POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
						pool_out_dim, NULL, img_buffer2);
	
	sha1sum("maxpool",img_buffer2, pool_out_dim * pool_out_dim * in_ch, digest);
	
	// test case 2
	BUFINIT(img_buffer1, 1024*800);
	in_ch = 24;
    in_dim = 18;
    pool_out_dim = 9;
	printf("case 2: input channel %d, input dimension %d, output dimension %d\n",in_ch, in_dim, pool_out_dim);
	
	arm_maxpool_q7_HWC(	img_buffer1, in_dim, in_ch, 
						POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
						pool_out_dim, NULL, img_buffer2);
	
	sha1sum("maxpool",img_buffer2, pool_out_dim * pool_out_dim * in_ch, digest);
	
	// test case 3
	BUFINIT(img_buffer1, 1024*800);
	in_ch = 16;
    in_dim = 66; 
    pool_out_dim = 33;
	printf("case 3: input channel %d, input dimension %d, output dimension %d\n",in_ch, in_dim, pool_out_dim);
	
	arm_maxpool_q7_HWC(	img_buffer1, in_dim, in_ch, 
						POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
						pool_out_dim, NULL, img_buffer2);
	
	sha1sum("maxpool",img_buffer2, pool_out_dim * pool_out_dim * in_ch, digest);
	
	// test case 4
	BUFINIT(img_buffer1, 1024*800);
	in_ch = 24;
    in_dim = 48; 
    pool_out_dim = 22;
	printf("case 4: input channel %d, input dimension %d, output dimension %d\n",in_ch, in_dim, pool_out_dim);
	
	arm_maxpool_q7_HWC(	img_buffer1, in_dim, in_ch, 
						POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
						pool_out_dim, NULL, img_buffer2);
	
	sha1sum("maxpool",img_buffer2, pool_out_dim * pool_out_dim * in_ch, digest);
	
	// test case 5
	BUFINIT(img_buffer1, 1024*800);
	in_ch = 24; 
    in_dim = 180;
    pool_out_dim = 90;
	printf("case 5: input channel %d, input dimension %d, output dimension %d\n",in_ch, in_dim, pool_out_dim);
	
	arm_maxpool_q7_HWC(	img_buffer1, in_dim, in_ch, 
						POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
						pool_out_dim, NULL, img_buffer2);
	
	sha1sum("maxpool",img_buffer2, pool_out_dim * pool_out_dim * in_ch, digest);
	
	// test case 6
	BUFINIT(img_buffer1, 1024*800);
	in_ch = 32;
    in_dim = 144;
    pool_out_dim = 72;
	printf("case 6: input channel %d, input dimension %d, output dimension %d\n",in_ch, in_dim, pool_out_dim);
	
	arm_maxpool_q7_HWC(	img_buffer1, in_dim, in_ch, 
						POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
						pool_out_dim, NULL, img_buffer2);
	
	sha1sum("maxpool",img_buffer2, pool_out_dim * pool_out_dim * in_ch, digest);
}