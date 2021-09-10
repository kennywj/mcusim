/***************************************************************************
 *  Copyright (C) 2018 Andes Technology Corporation                        *
 *  All rights reserved.                                                   *
 ***************************************************************************/

/** @file*/
/* VGG 19 uses 3 X 3 filters only */
#define KER_DIM 3
#define KER_PADDING 1
#define KER_STRIDE 1
/* VGG 19 uses 2 X 2 for Pooling layer */
#define POOL_KER_DIM 2
#define POOL_STRIDE 2
#define POOL_PADDING 0

#define CONV11_IM_DIM 224
#define CONV11_IM_CH 3
#define CONV11_OUT_CH 64
#define CONV11_OUT_DIM 224

#define CONV12_IM_DIM 224
#define CONV12_IM_CH 64
#define CONV12_OUT_CH 64
#define CONV12_OUT_DIM 224

#define POOL1_OUT_DIM 112

#define CONV21_IM_DIM 112
#define CONV21_IM_CH 64
#define CONV21_OUT_CH 128
#define CONV21_OUT_DIM 112

#define CONV22_IM_DIM 112
#define CONV22_IM_CH 128
#define CONV22_OUT_CH 128
#define CONV22_OUT_DIM 112

#define POOL2_OUT_DIM 56

#define CONV31_IM_DIM 56
#define CONV31_IM_CH 128
#define CONV31_OUT_CH 256
#define CONV31_OUT_DIM 56

#define CONV32_IM_DIM 56
#define CONV32_IM_CH 256
#define CONV32_OUT_CH 256
#define CONV32_OUT_DIM 56

#define CONV33_IM_DIM 56
#define CONV33_IM_CH 256
#define CONV33_OUT_CH 256
#define CONV33_OUT_DIM 56

#define CONV34_IM_DIM 56
#define CONV34_IM_CH 256
#define CONV34_OUT_CH 256
#define CONV34_OUT_DIM 56

#define POOL3_OUT_DIM 28

#define CONV41_IM_DIM 28
#define CONV41_IM_CH 256
#define CONV41_OUT_CH 512
#define CONV41_OUT_DIM 28

#define CONV42_IM_DIM 28
#define CONV42_IM_CH 512
#define CONV42_OUT_CH 512
#define CONV42_OUT_DIM 28

#define CONV43_IM_DIM 28
#define CONV43_IM_CH 512
#define CONV43_OUT_CH 512
#define CONV43_OUT_DIM 28

#define CONV44_IM_DIM 28
#define CONV44_IM_CH 512
#define CONV44_OUT_CH 512
#define CONV44_OUT_DIM 28

#define POOL4_OUT_DIM 14

#define CONV51_IM_DIM 14
#define CONV51_IM_CH 512
#define CONV51_OUT_CH 512
#define CONV51_OUT_DIM 14

#define CONV52_IM_DIM 14
#define CONV52_IM_CH 512
#define CONV52_OUT_CH 512
#define CONV52_OUT_DIM 14

#define CONV53_IM_DIM 14
#define CONV53_IM_CH 512
#define CONV53_OUT_CH 512
#define CONV53_OUT_DIM 14

#define CONV54_IM_DIM 14
#define CONV54_IM_CH 512
#define CONV54_OUT_CH 512
#define CONV54_OUT_DIM 14

#define POOL5_OUT_DIM 7

#define FC6_DIM 7*7*512
#define FC6_IM_DIM 1
#define FC6_IM_CH 1
#define FC6_OUT 4096

#define FC7_DIM 4096
#define FC7_IM_DIM 1
#define FC7_IM_CH 1
#define FC7_OUT 4096

#define FC8_DIM 4096
#define FC8_IM_DIM 1
#define FC8_IM_CH 1
#define FC8_OUT 1000

// fake one
#define BIAS_LSHIFT 3
#define OUT_RSHIFT 4
#define INPUT_MEAN_SHIFT {104,117,124}
#define INPUT_RIGHT_SHIFT {8,8,8}

