
.global image_start
.type image_start, @object

.global conv1_1_weight_start  
.global conv1_1_bias_start    
.global conv1_2_weight_start  
.global conv1_2_bias_start    

.global conv2_1_weight_start  
.global conv2_1_bias_start    
.global conv2_2_bias_start    
.global conv2_2_weight_start  

.global conv3_1_weight_start  
.global conv3_1_bias_start    
.global conv3_2_weight_start  
.global conv3_2_bias_start    
.global conv3_3_weight_start  
.global conv3_3_bias_start    
.global conv3_4_weight_start  
.global conv3_4_bias_start    

.global conv4_1_weight_start  
.global conv4_1_bias_start    
.global conv4_2_weight_start  
.global conv4_2_bias_start    
.global conv4_3_weight_start  
.global conv4_3_bias_start    
.global conv4_4_weight_start  
.global conv4_4_bias_start    

.global conv5_1_weight_start  
.global conv5_1_bias_start    
.global conv5_2_weight_start  
.global conv5_2_bias_start    
.global conv5_3_weight_start  
.global conv5_3_bias_start    
.global conv5_4_weight_start
.global conv5_4_bias_start    

.global fc6_weight_start
.global fc6_bias_start
.global fc7_weight_start
.global fc7_bias_start
.global fc8_weight_start
.global fc8_bias_start

.type conv1_1_weight_start, @object
.type conv1_1_bias_start, @object
.type conv1_2_weight_start, @object
.type conv1_2_bias_start, @object

.type conv2_1_weight_start, @object
.type conv2_1_bias_start, @object
.type conv2_2_bias_start, @object
.type conv2_2_weight_start, @object

.type conv3_1_weight_start, @object
.type conv3_1_bias_start, @object
.type conv3_2_weight_start, @object
.type conv3_2_bias_start, @object
.type conv3_3_weight_start, @object
.type conv3_3_bias_start, @object
.type conv3_4_weight_start, @object
.type conv3_4_bias_start, @object

.type conv4_1_weight_start, @object
.type conv4_1_bias_start, @object
.type conv4_2_weight_start, @object
.type conv4_2_bias_start, @object
.type conv4_3_weight_start, @object
.type conv4_3_bias_start, @object
.type conv4_4_weight_start, @object
.type conv4_4_bias_start, @object

.type conv5_1_weight_start, @object
.type conv5_1_bias_start, @object
.type conv5_2_weight_start, @object
.type conv5_2_bias_start, @object
.type conv5_3_weight_start, @object
.type conv5_3_bias_start, @object
.type conv5_4_weight_start, @object
.type conv5_4_bias_start, @object

.type fc6_weight_start, @object
.type fc6_bias_start, @object
.type fc7_weight_start, @object
.type fc7_bias_start, @object
.type fc8_weight_start, @object
.type fc8_bias_start, @object

.section .bindata , "a", @progbits
.align 2
image_start:
    .incbin "./nn/vgg_wb_bin/image.bin"

.align 2
conv1_1_weight_start:
    .incbin "./nn/vgg_wb_bin/conv1_1_weight.bin"

.align 2
conv1_1_bias_start:
    .incbin "./nn/vgg_wb_bin/conv1_1_bias.bin"

.align 2
conv1_2_weight_start:
    .incbin "./nn/vgg_wb_bin/conv1_2_weight.bin"

.align 2
conv1_2_bias_start:
    .incbin "./nn/vgg_wb_bin/conv1_2_bias.bin"

.align 2
conv2_1_weight_start:
    .incbin "./nn/vgg_wb_bin/conv2_1_weight.bin"

.align 2
conv2_1_bias_start:
    .incbin "./nn/vgg_wb_bin/conv2_1_bias.bin"

.align 2
conv2_2_weight_start:
    .incbin "./nn/vgg_wb_bin/conv2_2_weight.bin"

.align 2
conv2_2_bias_start:
    .incbin "./nn/vgg_wb_bin/conv2_2_bias.bin"

.align 2
conv3_1_weight_start:
    .incbin "./nn/vgg_wb_bin/conv3_1_weight.bin"

.align 2
conv3_1_bias_start:
    .incbin "./nn/vgg_wb_bin/conv3_1_bias.bin"

.align 2
conv3_2_weight_start:
    .incbin "./nn/vgg_wb_bin/conv3_2_weight.bin"

.align 2
conv3_2_bias_start:
    .incbin "./nn/vgg_wb_bin/conv3_2_bias.bin"

.align 2
conv3_3_weight_start:
    .incbin "./nn/vgg_wb_bin/conv3_3_weight.bin"

.align 2
conv3_3_bias_start:
    .incbin "./nn/vgg_wb_bin/conv3_3_bias.bin"

.align 2
conv3_4_weight_start:
    .incbin "./nn/vgg_wb_bin/conv3_4_weight.bin"

.align 2
conv3_4_bias_start:
    .incbin "./nn/vgg_wb_bin/conv3_4_bias.bin"

.align 2
conv4_1_weight_start:
    .incbin "./nn/vgg_wb_bin/conv4_1_weight.bin"

.align 2
conv4_1_bias_start:
    .incbin "./nn/vgg_wb_bin/conv4_1_bias.bin"

.align 2
conv4_2_weight_start:
    .incbin "./nn/vgg_wb_bin/conv4_2_weight.bin"

.align 2
conv4_2_bias_start:
    .incbin "./nn/vgg_wb_bin/conv4_2_bias.bin"

.align 2
conv4_3_weight_start:
    .incbin "./nn/vgg_wb_bin/conv4_3_weight.bin"

.align 2
conv4_3_bias_start:
    .incbin "./nn/vgg_wb_bin/conv4_3_bias.bin"

.align 2
conv4_4_weight_start:
    .incbin "./nn/vgg_wb_bin/conv4_4_weight.bin"

.align 2
conv4_4_bias_start:
    .incbin "./nn/vgg_wb_bin/conv4_4_bias.bin"

.align 2
conv5_1_weight_start:
    .incbin "./nn/vgg_wb_bin/conv5_1_weight.bin"

.align 2
conv5_1_bias_start:
    .incbin "./nn/vgg_wb_bin/conv5_1_bias.bin"

.align 2
conv5_2_weight_start:
    .incbin "./nn/vgg_wb_bin/conv5_2_weight.bin"

.align 2
conv5_2_bias_start:
    .incbin "./nn/vgg_wb_bin/conv5_2_bias.bin"

.align 2
conv5_3_weight_start:
    .incbin "./nn/vgg_wb_bin/conv5_3_weight.bin"

.align 2
conv5_3_bias_start:
    .incbin "./nn/vgg_wb_bin/conv5_3_bias.bin"

.align 2
conv5_4_weight_start:
    .incbin "./nn/vgg_wb_bin/conv5_4_weight.bin"

.align 2
conv5_4_bias_start:
    .incbin "./nn/vgg_wb_bin/conv5_4_bias.bin"

.align 2
fc6_weight_start:
    .incbin "./nn/vgg_wb_bin/fc6_weight.bin"

.align 2
fc6_bias_start:
    .incbin "./nn/vgg_wb_bin/fc6_bias.bin"

.align 2
fc7_weight_start:
    .incbin "./nn/vgg_wb_bin/fc7_weight.bin"

.align 2
fc7_bias_start:
    .incbin "./nn/vgg_wb_bin/fc7_bias.bin"

.align 2
fc8_weight_start:
    .incbin "./nn/vgg_wb_bin/fc8_weight.bin"

.align 2
fc8_bias_start:
    .incbin "./nn/vgg_wb_bin/fc8_bias.bin"

