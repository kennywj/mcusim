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
#include <assert.h>
#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "cmd.h"
#include "nn_test.h"

#if 1

#define POOL_KER_DIM 2
#define POOL_STRIDE 2
#define POOL_PADDING 0

q7_t scratch_buffer[1024 * 1600] __attribute__ ((aligned(64*(0x1<<10))));

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



#else


/**
 * @brief           This is a max pooling function test.
 *  @param[in]      maxpool parameters structure
 *	@param[in]      test configure parameters structure
 */
int test_maxpool(struct _maxpooling_ *mp, struct _config_ *cfg)
{
	unsigned char digest[20];
	int odim, in_size, out_size;
	
    memset(scratch_buffer, 0, sizeof(q7_t)*sizeof(scratch_buffer));
    q7_t *img_buffer1; 
    q7_t *img_buffer2; 
	
	odim = (mp->dim - mp->ker + 2 * mp->pad)/mp->stride + 1;
	in_size = mp->dim * mp->dim * mp->ch;
	out_size = odim * odim * mp->ch;
	
	printf("\n### Start test ###\n"
		"   Input feature %dx%d, channe1 %d, pool kernel %dx%d, pad %d, stride %d, in size %d\n"
		"   Output feature %dx%d, out size %d\n\n",
		mp->dim, mp->dim, mp->ch, mp->ker, mp->ker, mp->pad, mp->stride, in_size, odim, odim, out_size);
	
	// Do ARM nn API to generate golden pattern
	img_buffer1 = scratch_buffer;
	img_buffer2 = img_buffer1 + (((in_size -1) | 0xf)+1);
	assert((img_buffer2 + out_size)<buffer_end);
	
	printf("   generate: in buffer %p, size 0x%x, out buffer %p, size 0x%x\n",
		img_buffer1, in_size, img_buffer2, out_size);
	gen_data(mp->dim, mp->ch, mp->ch, 0, cfg->increase, cfg->data, 0, img_buffer1);
	if (cfg->dump)
		dump_array("original input feature map", img_buffer1, mp->dim, mp->ch);
	sha1sum("original",img_buffer1, mp->dim * mp->dim * mp->ch, digest);
	
    arm_maxpool_q7_HWC(img_buffer1, mp->dim, mp->ch, mp->ker, mp->pad, mp->stride, odim, NULL, img_buffer2);
	if (cfg->dump)
		dump_array("arm_maxpool_q7_HWC", img_buffer2, odim, mp->ch);
	sha1sum("arm_maxpool",img_buffer2, odim * odim * mp->ch, digest);
	
	printf("### end test ###");
    return 0;
}

/**
  * @brief  do pool test and calculate sha1 digest
  * @param  argc
  * @param  argv
  * @retval None
  */
void cmd_pool(int argc, char* argv[])
{
	static struct _config_ cfg ={1, 1, 1, 1, 0, 0, 0, 0};
	static struct _maxpooling_ mp = {5, 1, 2, 1, 1};
	int c;
	// start a thread to do ssl client 
	while((c=getopt(argc, argv, "?hi:p:d:s:m:")) != -1)
    {
        switch(c)
        {
		case 'd':
			cfg.dump = (strcmp(optarg,on_off[1])==0)?1:0;
			break;
		case 's':
			cfg.sha1 = (strcmp(optarg,on_off[1])==0)?1:0;
			break;
		case 'i':
			cfg.data = atoi(optarg);
			break;
		case 'p':
			cfg.pad_val = atoi(optarg);
			break;
		case 'm':
			cfg.increase = atoi(optarg);
			break;
		case 'h':
		case '?':
			printf("Configuration: do sha1 hash: %s, dump contain of buffer:%s\n   initial data pattern: %d, data pattern increase:%d\n",
				on_off[cfg.sha1], on_off[cfg.dump], cfg.data, cfg.increase);
			printf("usage: %s\n",curr_cmd->usage);
			goto end_do_maxpool_test;
        }
    }   // end while
	
	c=0;
	while (optind < argc)
	{
		switch(c)
		{
			case 0:
				mp.dim = atoi(argv[optind]);
			break;
			case 1:
				mp.ch = atoi(argv[optind]);
			break;
			case 2:
				mp.ker = atoi(argv[optind]);
			break;
			case 3:
				mp.pad = atoi(argv[optind]);
			break;
			case 4:
				mp.stride = atoi(argv[optind]);
			break;
			default:
				goto start_do_maxpool_test;
		}
		optind++;
		c++;
	}
start_do_maxpool_test:
	test_maxpool(&mp, &cfg);
end_do_maxpool_test:
	return;
}

#endif