/* states */
#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "cmd.h"
#include "nn_test.h"

/**
 * @brief           This is a max pooling function test.
 *  @param[in]      maxpool parameters structure
 *	@param[in]      test configure parameters structure
 */
int test_maxpool(struct _maxpooling_ *mp, struct _config_ *cfg)
{
	unsigned char digest[2][20];
	int odim, extend_ch, in_size, out_size;
	
    memset(scratch_buffer, 0, sizeof(q7_t)*sizeof(scratch_buffer));
    q7_t *img_buffer1; 
    q7_t *img_buffer2; 
	q7_t *img_buffer3;
	q7_t *img_buffer4;
	
	odim = (mp->dim - mp->ker + 2 * mp->pad)/mp->stride + 1;
	in_size = (mp->dim + 2 * mp->pad) * (mp->dim +2 * mp->pad) * mp->ch;
	out_size = odim * odim * mp->ch;
	
	printf("\n### Start test ###\n   Input feature %dx%d, channe1 %d, "
		"pool kernel %dx%d, pad %d, stride %d, in size %d,\n   Output feature %dx%d, out size %d\n\n",
		mp->dim, mp->dim, mp->ch, mp->ker, mp->ker, mp->pad, mp->stride, in_size, odim, odim, out_size);
	
	// Do libnn API to generate golden pattern
	
	img_buffer1 = scratch_buffer;
	img_buffer2 = img_buffer1 + (((in_size -1) | 0xf)+1);
	img_buffer3 = img_buffer2 + (((in_size -1) | 0xf)+1);
	assert((img_buffer3 + out_size)<buffer_end);
	
	printf("   generate: in buffer %p, size 0x%x, out buffer %p, size 0x%x\n",img_buffer1, in_size, img_buffer3, out_size);
	gen_data(mp,cfg,img_buffer1,mp->ch);
	memcpy(img_buffer2, img_buffer1, in_size);
	if (cfg->dump)
		dump_array("original input feature map", img_buffer1, (mp->dim + mp->pad *2), mp->ch);
	sha1sum("original",img_buffer1, mp->dim * mp->dim * mp->ch, digest);
	
    arm_maxpool_q7_HWC(img_buffer1, mp->dim, mp->ch, mp->ker, mp->pad, mp->stride, odim, NULL, img_buffer3);
	if (cfg->dump)
		dump_array("arm_maxpool_q7_HWC", img_buffer3, odim, mp->ch);
	sha1sum("arm_maxpool",img_buffer2, odim * odim * mp->ch, digest[0]);

	
	img_buffer4 = img_buffer3 + (((out_size -1) | 0xf)+1);
	odim = ((mp->dim + 2 * mp->pad) - mp->ker)/mp->stride + 1;
	out_size = odim * odim * mp->ch;
	assert((img_buffer4 + out_size)<buffer_end);
	
	printf("\n### Next test ###\n   Input feature %dx%d, channe1 %d, "
		"pool kernel %dx%d, pad %d, stride %d, in size %d,\n   Output feature %dx%d, out size %d\n\n",
		mp->dim+mp->pad*2, mp->dim+mp->pad*2, mp->ch, mp->ker, mp->ker, 0, mp->stride, in_size, odim, odim, out_size);
	printf("   generate: in buffer %p, size 0x%x, out buffer %p, size 0x%x\n",img_buffer2, in_size, img_buffer4, out_size);	if (cfg->dump)
	if (cfg->dump)
		dump_array("original input feature map", img_buffer2, (mp->dim + mp->pad *2), mp->ch);
	
	arm_maxpool_q7_HWC(img_buffer2, mp->dim+mp->pad*2, mp->ch, mp->ker, 0, mp->stride, odim, NULL, img_buffer4);
	if (cfg->dump)
		dump_array("arm_maxpool_q7_HWC", img_buffer4, odim, mp->ch);
	sha1sum("arm_maxpool",img_buffer4, odim * odim * mp->ch, digest[0]);
    return 0;
}

/**
  * @brief  do pool test and calculate sha1 digest
  * @param  argc
  * @param  argv
  * @retval None
  */
void cmd_testpool(int argc, char* argv[])
{
	unsigned char digest[20];
	static struct _config_ cfg ={1, 1, 1, 0, 1, 0, 0, 0};
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