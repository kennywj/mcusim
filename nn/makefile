#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build/nn
LIB = ../build/libnn.a
INCS = -I./inc -I./json -I../FreeRTOS_Posix -I../FreeRTOS_Posix/FreeRTOS_Kernel/include \
	-I../FreeRTOS_Posix -I../shell -I../include
VPATH = .:./src/ActivationFunctions:./src/ConvolutionFunctions:./src/FullyConnectedFunctions: \
		./src/NNSupportFunctions:./src/PoolingFunctions:./src/SoftmaxFunctions
SRCS = port_nn.c \
	arm_nn_activations_q7.c arm_relu_q7.c arm_nn_mat_mult_kernel_q7_q15_reordered.c \
	arm_nn_mat_mult_kernel_q7_q15.c arm_convolve_1x1_HWC_q7_fast_nonsquare.c arm_convolve_HWC_q7_fast.c \
	arm_depthwise_separable_conv_HWC_q7_nonsquare.c arm_convolve_HWC_q7_fast_nonsquare.c	\
	arm_convolve_HWC_q7_RGB.c arm_convolve_HWC_q7_basic.c arm_depthwise_separable_conv_HWC_q7.c \
	arm_fully_connected_q7.c arm_fully_connected_q7_opt.c arm_nntables.c arm_q7_to_q15_no_shift.c \
	arm_q7_to_q15_reordered_no_shift.c arm_pool_q7_HWC.c arm_softmax_q7.c

OBJS = $(patsubst %.c,$(ODIR)/%.o,$(SRCS))

.PHONY: directories clean depend

all: directories $(LIB)

directories: $(ODIR)

$(ODIR):
	$(MKDIR_P) $(ODIR)
	
$(LIB): $(OBJS)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"
   
$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@  

clean:
	rm -f $(OBJS) $(LIB) *.bak

depend: $(SRCS)
	makedepend $(INCS) $<
