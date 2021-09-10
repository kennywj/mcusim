#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "vgg19_parameter.h"
#include "sha1.h"
#include "cmd.h"

extern uint8_t image_start;

extern q7_t conv1_1_weight_start;
extern q7_t conv1_1_bias_start;
extern q7_t conv1_2_weight_start;
extern q7_t conv1_2_bias_start;

extern q7_t conv2_1_weight_start;
extern q7_t conv2_1_bias_start;
extern q7_t conv2_2_bias_start;
extern q7_t conv2_2_weight_start;

extern q7_t conv3_1_weight_start;
extern q7_t conv3_1_bias_start;
extern q7_t conv3_2_weight_start;
extern q7_t conv3_2_bias_start;
extern q7_t conv3_3_weight_start;
extern q7_t conv3_3_bias_start;
extern q7_t conv3_4_weight_start;
extern q7_t conv3_4_bias_start;

extern q7_t conv4_1_weight_start;
extern q7_t conv4_1_bias_start;
extern q7_t conv4_2_weight_start;
extern q7_t conv4_2_bias_start;
extern q7_t conv4_3_weight_start;
extern q7_t conv4_3_bias_start;
extern q7_t conv4_4_weight_start;
extern q7_t conv4_4_bias_start;

extern q7_t conv5_1_weight_start;
extern q7_t conv5_1_bias_start;
extern q7_t conv5_2_weight_start;
extern q7_t conv5_2_bias_start;
extern q7_t conv5_3_weight_start;
extern q7_t conv5_3_bias_start;
extern q7_t conv5_4_weight_start;
extern q7_t conv5_4_bias_start;

extern q7_t fc6_weight_start;
extern q7_t fc6_bias_start;
extern q7_t fc7_weight_start;
extern q7_t fc7_bias_start;
extern q7_t fc8_weight_start;
extern q7_t fc8_bias_start;

// include the input and weights
static q7_t *conv11_wt = &conv1_1_weight_start;
static q7_t *conv11_bias = &conv1_1_bias_start;
static q7_t *conv12_wt = &conv1_2_weight_start;
static q7_t *conv12_bias = &conv1_2_bias_start;

static q7_t *conv21_wt = &conv2_1_weight_start;
static q7_t *conv21_bias = &conv2_1_bias_start;
static q7_t *conv22_wt = &conv2_2_weight_start;
static q7_t *conv22_bias = &conv2_2_bias_start;

static q7_t *conv31_wt = &conv3_1_weight_start;
static q7_t *conv31_bias = &conv3_1_bias_start;
static q7_t *conv32_wt = &conv3_2_weight_start;
static q7_t *conv32_bias = &conv3_2_bias_start;
static q7_t *conv33_wt = &conv3_3_weight_start;
static q7_t *conv33_bias = &conv3_3_bias_start;
static q7_t *conv34_wt = &conv3_4_weight_start;
static q7_t *conv34_bias = &conv3_4_bias_start;

static q7_t *conv41_wt = &conv4_1_weight_start;
static q7_t *conv41_bias = &conv4_1_bias_start;
static q7_t *conv42_wt = &conv4_2_weight_start;
static q7_t *conv42_bias = &conv4_2_bias_start;
static q7_t *conv43_wt = &conv4_3_weight_start;
static q7_t *conv43_bias = &conv4_3_bias_start;
static q7_t *conv44_wt = &conv4_4_weight_start;
static q7_t *conv44_bias = &conv4_4_bias_start;

static q7_t *conv51_wt = &conv5_1_weight_start;
static q7_t *conv51_bias = &conv5_1_bias_start;
static q7_t *conv52_wt = &conv5_2_weight_start;
static q7_t *conv52_bias = &conv5_2_bias_start;
static q7_t *conv53_wt = &conv5_3_weight_start;
static q7_t *conv53_bias = &conv5_3_bias_start;
static q7_t *conv54_wt = &conv5_4_weight_start;
static q7_t *conv54_bias = &conv5_4_bias_start;

static q7_t *fc6_wt = &fc6_weight_start;
static q7_t *fc6_bias = &fc6_bias_start;

static q7_t *fc7_wt = &fc7_weight_start;
static q7_t *fc7_bias = &fc7_bias_start;

static q7_t *fc8_wt = &fc8_weight_start;
static q7_t *fc8_bias = &fc8_bias_start;

/* Here the image_data should be the raw uint8 type RGB image in [RGB, RGB, RGB ... RGB] format */
static uint8_t *image_data = &image_start;

//vector buffer: max(im2col buffer,average pool buffer, fully connected buffer)
q7_t rgb_wt_buffer[CONV11_OUT_CH * (3 * KER_DIM * KER_DIM + 1)];
q7_t col_buffer[CONV11_OUT_CH * KER_DIM * KER_DIM *4];

// ofmap buffer need more 16*out_ch -- fix me
q7_t scratch_buffer[224 * 224 * 64 * 6 + 10] __attribute__ ((aligned(64*(0x1<<10))));

/**
  * @brief  do vgg19 test and calculate sha1 digest
  * @param  argc
  * @param  argv
  * @retval None
  */
void cmd_vgg19(int argc, char* argv[])
{
	unsigned char digest[20];
	
    /* input pre-processing */
    int mean_data[3] = INPUT_MEAN_SHIFT;
    unsigned int scale_data[3] = INPUT_RIGHT_SHIFT;

    memset(scratch_buffer, 0, sizeof(q7_t)*sizeof(scratch_buffer));
    q7_t *img_buffer1 = scratch_buffer;
    q7_t *img_buffer2 = &scratch_buffer[224*224*64*3];
    //q7_t *output_data = &scratch_buffer[224*224*64*6];


    int i;
    for (i = 0; i < 224*224*3; i += 3) {
        img_buffer2[i] = (q7_t)__SSAT(((((int)image_data[i]   - mean_data[0]) << 7) + (0x1 << (scale_data[0] - 1))) >> scale_data[0], 8);
        img_buffer2[i+1] = (q7_t)__SSAT(((((int)image_data[i+1] - mean_data[1]) << 7) + (0x1 << (scale_data[1] - 1))) >> scale_data[1], 8);
        img_buffer2[i+2] = (q7_t)__SSAT(((((int)image_data[i+2] - mean_data[2]) << 7) + (0x1 << (scale_data[2] - 1))) >> scale_data[2], 8);
    }

    // conv11 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_basic(img_buffer2, CONV11_IM_DIM, CONV11_IM_CH, conv11_wt, CONV11_OUT_CH, KER_DIM, KER_PADDING, KER_STRIDE, conv11_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV11_OUT_DIM, (q15_t *) col_buffer, rgb_wt_buffer);
	sha1sum("conv1_1",img_buffer1,CONV11_OUT_DIM*CONV11_OUT_DIM*CONV11_OUT_CH,digest);
	
    // conv12 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV12_IM_DIM, CONV12_IM_CH, conv12_wt, CONV12_OUT_CH, KER_DIM, KER_PADDING, KER_STRIDE, conv12_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV12_OUT_DIM, (q15_t *) col_buffer, NULL);
    sha1sum("conv1_2",img_buffer2,CONV12_OUT_DIM*CONV12_OUT_DIM*CONV12_OUT_CH,digest);

    // pool1 img_buffer2 -> img_buffer1
    arm_maxpool_q7_HWC(img_buffer2, CONV12_OUT_DIM, CONV12_OUT_CH, POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, POOL1_OUT_DIM, NULL, img_buffer1);
    sha1sum("pool1",img_buffer1,POOL1_OUT_DIM*POOL1_OUT_DIM*CONV12_OUT_CH,digest);

    // conv21 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV21_IM_DIM, CONV21_IM_CH, conv21_wt, CONV21_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv21_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV21_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv2_1",img_buffer2,CONV21_OUT_DIM*CONV21_OUT_DIM*CONV21_OUT_CH,digest);

    // conv22 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV22_IM_DIM, CONV22_IM_CH, conv22_wt, CONV22_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv22_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV22_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv2_2",img_buffer1,CONV22_OUT_DIM*CONV22_OUT_DIM*CONV22_OUT_CH,digest);

    // pool2 img_buffer1 -> img_buffer2
    arm_maxpool_q7_HWC(img_buffer1, CONV22_OUT_DIM, CONV22_OUT_CH, POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
		POOL2_OUT_DIM, NULL, img_buffer2);
	sha1sum("pool2",img_buffer2,POOL2_OUT_DIM*POOL2_OUT_DIM*CONV22_OUT_CH,digest);

    // conv31 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV31_IM_DIM, CONV31_IM_CH, conv31_wt, CONV31_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv31_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV31_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv3_1",img_buffer1,CONV31_OUT_DIM*CONV31_OUT_DIM*CONV31_OUT_CH,digest);

    // conv32 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV32_IM_DIM, CONV32_IM_CH, conv32_wt, CONV32_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv32_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV32_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv3_2",img_buffer2,CONV32_OUT_DIM*CONV32_OUT_DIM*CONV32_OUT_CH,digest);

    // conv33 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV33_IM_DIM, CONV33_IM_CH, conv33_wt, CONV33_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv33_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV33_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv3_3",img_buffer1,CONV33_OUT_DIM*CONV33_OUT_DIM*CONV33_OUT_CH,digest);

    // conv34 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV34_IM_DIM, CONV34_IM_CH, conv34_wt, CONV34_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv34_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV34_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv3_4",img_buffer2,CONV34_OUT_DIM*CONV34_OUT_DIM*CONV34_OUT_CH,digest);

    // pool3 img_buffer2 -> img_buffer1
    arm_maxpool_q7_HWC(img_buffer2, CONV34_OUT_DIM, CONV34_OUT_CH, POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
		POOL3_OUT_DIM, NULL, img_buffer1);
	sha1sum("pool3",img_buffer1,POOL3_OUT_DIM*POOL3_OUT_DIM*CONV34_OUT_CH,digest);

    // conv41 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV41_IM_DIM, CONV41_IM_CH, conv41_wt, CONV41_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv41_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV41_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv4_1",img_buffer2,CONV41_OUT_DIM*CONV41_OUT_DIM*CONV41_OUT_CH,digest);

    // conv42 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV42_IM_DIM, CONV42_IM_CH, conv42_wt, CONV42_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv42_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV42_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv4_2",img_buffer1,CONV42_OUT_DIM*CONV42_OUT_DIM*CONV42_OUT_CH,digest);

    // conv43 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV43_IM_DIM, CONV43_IM_CH, conv43_wt, CONV43_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv43_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV43_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv4_3",img_buffer2,CONV43_OUT_DIM*CONV43_OUT_DIM*CONV43_OUT_CH,digest);

    // conv44 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV44_IM_DIM, CONV44_IM_CH, conv44_wt, CONV44_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv44_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV44_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv4_4",img_buffer1,CONV44_OUT_DIM*CONV44_OUT_DIM*CONV44_OUT_CH,digest);

    // pool4 img_buffer1 -> img_buffer2
    arm_maxpool_q7_HWC(img_buffer1, CONV44_OUT_DIM, CONV44_OUT_CH, POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
		POOL4_OUT_DIM, NULL, img_buffer2);
	sha1sum("pool4",img_buffer2,POOL4_OUT_DIM*POOL4_OUT_DIM*CONV44_OUT_CH,digest);

    // conv51 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV51_IM_DIM, CONV51_IM_CH, conv51_wt, CONV51_OUT_CH, KER_DIM, KER_PADDING,
		KER_STRIDE, conv51_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV51_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv5_1",img_buffer1,CONV51_OUT_DIM*CONV51_OUT_DIM*CONV51_OUT_CH,digest);

    // conv52 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV52_IM_DIM, CONV52_IM_CH, conv52_wt, CONV52_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv52_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV52_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv5_2",img_buffer2,CONV52_OUT_DIM*CONV52_OUT_DIM*CONV52_OUT_CH,digest);

    // conv53 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV53_IM_DIM, CONV53_IM_CH, conv53_wt, CONV53_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv53_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer1, CONV53_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv5_3",img_buffer1,CONV53_OUT_DIM*CONV53_OUT_DIM*CONV53_OUT_CH,digest);

    // conv54 img_buffer1 -> img_buffer2
    arm_convolve_HWC_q7_fast(img_buffer1, CONV54_IM_DIM, CONV54_IM_CH, conv54_wt, CONV54_OUT_CH, KER_DIM, KER_PADDING, 
		KER_STRIDE, conv54_bias, BIAS_LSHIFT, OUT_RSHIFT, img_buffer2, CONV54_OUT_DIM, (q15_t *) col_buffer, NULL);
	sha1sum("conv5_4",img_buffer2,CONV54_OUT_DIM*CONV54_OUT_DIM*CONV54_OUT_CH,digest);

    // pool5 img_buffer2 -> img_buffer1
    arm_maxpool_q7_HWC(img_buffer2, CONV54_OUT_DIM, CONV54_OUT_CH, POOL_KER_DIM, POOL_PADDING, POOL_STRIDE, 
		POOL5_OUT_DIM, NULL, img_buffer1);
	sha1sum("pool5",img_buffer1,POOL5_OUT_DIM*POOL5_OUT_DIM*CONV54_OUT_CH,digest);

    // fc6 img_buffer1 -> img_buffer2
    // to sync the result with DLA, so use the nds_nn_fc_q7() 
    arm_fully_connected_q7(img_buffer1, fc6_wt, FC6_DIM, FC6_OUT, BIAS_LSHIFT, OUT_RSHIFT, fc6_bias, img_buffer2, 
		(q15_t *) col_buffer);
    arm_relu_q7(img_buffer2, FC6_OUT);
	sha1sum("fc6+relu",img_buffer2,FC6_OUT,digest);

    // fc7 img_buffer2 -> img_buffer1
    // to sync the result with DLA, so use the nds_nn_fc_q7() 
    arm_fully_connected_q7(img_buffer2, fc7_wt, FC7_DIM, FC7_OUT, BIAS_LSHIFT, OUT_RSHIFT, fc7_bias, img_buffer1, 
		(q15_t *) col_buffer);
    arm_relu_q7(img_buffer1, FC7_OUT);
	sha1sum("fc7+relu",img_buffer1,FC7_OUT,digest);

    // fc8 img_buffer1 -> img_buffer2
    // to sync the result with DLA, so use the nds_nn_fc_q7() 
    arm_fully_connected_q7(img_buffer1, fc8_wt, FC8_DIM, FC8_OUT, BIAS_LSHIFT, OUT_RSHIFT, fc8_bias, img_buffer2, 
		(q15_t *) col_buffer);
    arm_relu_q7(img_buffer2, FC8_OUT);
	sha1sum("fc8+relu",img_buffer2,FC8_OUT,digest);

    // softmax
    arm_softmax_q7(img_buffer2, 1000, img_buffer2);
	sha1sum("sofmax",img_buffer2,1000,digest);
	
    printf("VGG 19 done\n");
}
