cmake_minimum_required(VERSION 2.6)
#message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(nn)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR})

add_library(nn STATIC port_nn.c 
	src/ActivationFunctions/arm_nn_activations_q15.c
	src/ActivationFunctions/arm_nn_activations_q7.c
	src/ActivationFunctions/arm_relu_q15.c
	src/ActivationFunctions/arm_relu_q7.c
	src/ConvolutionFunctions/arm_convolve_1x1_HWC_q7_fast_nonsquare.c
	src/ConvolutionFunctions/arm_convolve_HWC_q7_fast.c
	src/ConvolutionFunctions/arm_depthwise_separable_conv_HWC_q7_nonsquare.c
	src/ConvolutionFunctions/arm_convolve_HWC_q15_basic.c
	src/ConvolutionFunctions/arm_convolve_HWC_q7_fast_nonsquare.c
	src/ConvolutionFunctions/arm_nn_mat_mult_kernel_q7_q15.c
	src/ConvolutionFunctions/arm_convolve_HWC_q15_fast.c
	src/ConvolutionFunctions/arm_convolve_HWC_q7_RGB.c
	src/ConvolutionFunctions/arm_nn_mat_mult_kernel_q7_q15_reordered.c
	src/ConvolutionFunctions/arm_convolve_HWC_q7_basic.c
	src/ConvolutionFunctions/arm_depthwise_separable_conv_HWC_q7.c
	src/FullyConnectedFunctions/arm_fully_connected_mat_q7_vec_q15.c
	src/FullyConnectedFunctions/arm_fully_connected_q15.c
	src/FullyConnectedFunctions/arm_fully_connected_q7.c
	src/FullyConnectedFunctions/arm_fully_connected_mat_q7_vec_q15_opt.c
	src/FullyConnectedFunctions/arm_fully_connected_q15_opt.c
	src/FullyConnectedFunctions/arm_fully_connected_q7_opt.c
	src/NNSupportFunctions/arm_nntables.c
	src/NNSupportFunctions/arm_q7_to_q15_no_shift.c
	src/NNSupportFunctions/arm_q7_to_q15_reordered_no_shift.c
	src/PoolingFunctions/arm_pool_q7_HWC.c
	src/SoftmaxFunctions/arm_softmax_q15.c
	src/SoftmaxFunctions/arm_softmax_q7.c)