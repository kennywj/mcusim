#ifndef _ANDLA_TEST_
#define _ANDLA_TEST_
#include "arm_math.h"
#include "arm_nnfunctions.h"
//#include "andla_nn_api.h"

struct _config_
{
	int sha1;		// result do hash and output hash
	int dump;		// dump output ?
	int data;		// data pattern
	int increase;	// increase or decrease data pattern
	int pad_val;	// padding data pattern
	int weight;		// weight for convlution
	int bias;		// bias for convlution
	int relu;		// relu function 0: none, 1: relue, 2 leak_relu
};


struct _maxpooling_
{
	unsigned short dim;		// input feature map dimension x, y
	unsigned short ch;		// input channel number
	unsigned short ker;		// pooling kernel dimension x, y
	unsigned char pad;		// padding size
	unsigned char stride;	// stride size
};

struct _conv_
{
	unsigned short in_dim;		// input feature map dimension x, y
	unsigned short in_ch;		// input channel number
	unsigned short out_ch;		// output channel number
	unsigned short ker;		// convlution kernel dimension x, y
	unsigned char pad;		// padding size
	unsigned char stride;	// stride size
};

extern const char *on_off[2];
extern q7_t scratch_buffer[1024 * 1600];
extern const q7_t *buffer_end;

extern int test_maxpool(struct _maxpooling_ *mp, struct _config_ *cfg);
extern int test_conv(struct _conv_ *cp, struct _config_ *cfg);

extern void dump_array(char *msg, q7_t *array, int dim, int ch);
extern void gen_data(int dim, int channel, int extend_ch, 
	int pad, int increase, int data, int pad_val, q7_t *buf);
extern void shorten_data(int dim, int ch, int extend_ch, q7_t *src, q7_t *dst);
extern void cmpmem(q7_t *buf1, q7_t *buf2, int size);

extern void nds_dcache_invalidate_range(unsigned long start, unsigned long end);

#endif
// end of _ANDLA_TEST_
