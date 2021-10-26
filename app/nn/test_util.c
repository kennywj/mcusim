#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sha1.h"
#include "nn_test.h"

const char *on_off[2]={"off","on"};
q7_t scratch_buffer[1024 * 1600];
const q7_t *buffer_end = & scratch_buffer[1024 * 1600];

/**
 * @brief           Dump 3D array, channel + 2D matrix
 * @param[in]       message 
 * @param[in]       dimension x, y, channel
 * @return          None
 */
void dump_array(char *msg, q7_t *array, int dim, int ch)
{
	int i,j,k;
	q7_t *cp=array;
	
	printf("%s dim=%dx%d, ch=%d\n",msg, dim, dim, ch);
	for (k=0;k<ch;k++)
	{	
		cp = array + k;
		printf("ch=%d\n",k);
		for(i=0;i<dim;i++)
		{
			printf("   ");
			for(j=0;j<dim;j++)
			{	
				printf("%3d ",*cp);
				cp += ch;
			}
			printf("\n");
		}
	}
	printf("\n");
}

/**
 * @brief           generate test data, WHC format
 * @param[in]       input feature map dimension dim * dim 
 * @param[in]       channel number
 * @param[in]       extend channel numbere
 * @param[in]       pad number
 * @param[in]       data pattern increase 
 * @param[in]       initil data pattern
 * @param[in]       pad value
 * @param[out]      genearated data buffer
 * @return          None
 */
void gen_data(int dim, int channel, int extend_ch, int pad, int increase, int data, int pad_val, q7_t *buf)
{
	if (pad)
	{
		// with pading process
		int c, x, y, val;
		val = data;
		for (c=0;c<channel;c++)
		{	
			for (y=0;y<(dim+pad*2);y++)
			{
				if (y < pad || y >= (dim+pad))
				{
					for (x=0; x<(dim+pad*2); x++)
						buf[(x + (y * (dim + pad *2))) * extend_ch + c] = pad_val;
					continue;
				}
				for (x=0; x<(dim+pad*2); x++)
				{
					if (x < pad || x >= (dim+pad))
					{
						buf[(x + (y * (dim + pad *2))) * extend_ch + c] = pad_val;
						continue;
					}
					buf[(x + (y * (dim + pad *2))) * extend_ch + c] = (q7_t)(val) & 0x7f;
					val += increase;
				}
			}
		}
	}
	else
	{
		// without pading process
		int c, i, j, val;
		val = data;
		for (c=0;c<channel;c++)
		{	
			for (i=0, j=0 ; i< (dim * dim) ; i++, j+=extend_ch)
			{
				buf[j+c] = (q7_t)(val) & 0x7f;
				val += increase;
			}
		}
	}
}
 
 
 /**
 * @brief           shorten feature maps, remove extend channel
 * @param[in]       dimension 
 * @param[in]       channel
 * @param[in]       extend channel
 * @param[in]       src buffer pointer
 * @param[in]       dst buffer pointer
 * @return          None
 */
 void shorten_data(int dim, int ch, int extend_ch, q7_t *src, q7_t *dst)
 {
	int i,j,c;
	q7_t *cp = dst;
	
	for (i=0, j=0 ; i< (dim*dim) ; i++, j+=extend_ch)
	{
		for (c=0;c<ch;c++)
			*cp++ = src[j+c];
	}
 }
 
 /**
 * @brief           memory compare, show difference
 * @param[in]       buf 1 pointer
 * @param[in]       buf 2 pointer
 * @param[in]       size
 * @return          None
 */
void cmpmem(q7_t *buf1, q7_t *buf2, int size)
{
	int count=0,i;
	
	printf("compare memory %p, %p, size %d\n",buf1, buf2, size);
	for (i=0;i<size; i++)
	{	
		if (buf1[i] != buf2[i])
		{	
			printf("[%04d], 0x%02x != 0x%02x,",i, buf1[i], buf2[i]);
			count ++;
			if ((count % 4)==0)
				printf("\n");
		}
	}
	printf("\n");
}

#if 0
#include "shell.h"
#include "andla_hw_conf.h"

extern q7_t scratch_buffer[1024 * 1600];
/**
 * @brief           To test the utility APIs: gen_data, dump_array, shorten_data
 * @param[in]       argc
 * @param[in]       argv
 * @return          0
 */
int do_util_test(int argc, char **argv)
{
	// default enables sha1 hash, enables dump dump data array, initial value 1, initial padding 0,  pattern, increase
	static struct _config_ cfg ={1, 1, 1, 0, 3};	
	static struct _maxpooling_ mp = {5, 1, 2, 0, 1};
	int c;
	if (argc <= 1)
	{
		printf("Usage: utiltest [<input dim> <channel> <pad number> <padding pattern>]\n");
		return 0;
	}
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
				mp.pad = atoi(argv[optind]);
			break;
			case 3:
				cfg.pad_val = atoi(argv[optind]);
			break;
			default:
				goto start_do_util_test;
		}
		optind++;
		c++;
	}
start_do_util_test:
	memset(scratch_buffer, 0, sizeof(q7_t)*sizeof(scratch_buffer));
    q7_t *img_buffer1; 
    q7_t *img_buffer2; 
	q7_t *img_buffer3; 
	q7_t *img_buffer4;

	printf("\nUnit Test: test the utility functions\n"
		"   input dimenstion %dx%d, channel %d, padding %d, padding data %d\n",
		mp.dim, mp.dim, mp.ch, mp.pad, cfg.pad_val);
	//unsigned char digest[2][20];
	int extend_ch, size1, size2;
	
	size1 = (mp.dim+mp.pad*2) * (mp.dim+mp.pad*2) * mp.ch;

	img_buffer1 = scratch_buffer;
	img_buffer2 = img_buffer1 + (((size1 -1) | 0xf)+1);
	assert(img_buffer2<buffer_end);
	
	printf("   gendata: in buffer %p, size 0x%x\n",img_buffer1, size1);
	gen_data(&mp,&cfg,img_buffer1,mp.ch);
	//sha1sum("1",img_buffer1, mp.dim * mp.dim * mp.ch, digest[0]);
	dump_array("",img_buffer1, (mp.dim + mp.pad*2) , mp.ch);
	
	extend_ch = ((mp.ch -1) | (Bus_Width_Byte-1))+1;
	size2 = (mp.dim+mp.pad*2) * (mp.dim+mp.pad*2) * extend_ch;
	img_buffer3 = img_buffer2 + (((size2-1)|0xf)+1);
	assert(img_buffer3<buffer_end);
	
	printf("   Extend channel: in buffer %p, size 0x%x\n",img_buffer2, size2);
	gen_data(&mp,&cfg,img_buffer2,extend_ch);
	//sha1sum("2",img_buffer2, mp.dim * mp.dim * extend_ch, digest[0]);
	dump_array("",img_buffer2, (mp.dim + mp.pad*2) , extend_ch);
	
	img_buffer4 = img_buffer3 + (((size1-1)|0xf)+1);
	assert(img_buffer4<buffer_end);
	
	printf("   shorten data: in buffer %p, size 0x%x\n",img_buffer3, size1);
	shorten_data((mp.dim + mp.pad *2), mp.ch, extend_ch, img_buffer2, img_buffer3);
	//sha1sum("3",img_buffer3, mp.dim * mp.dim * mp.ch, digest[0]);
	dump_array("",img_buffer3, (mp.dim + mp.pad*2) , mp.ch);
	return 0;
}

CLI_CMD(utiltest, do_util_test, 999);
#endif