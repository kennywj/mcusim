################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fatfs/src/diskio.c \
../fatfs/src/ff.c   \
../fatfs/src/ffsystem.c

OBJS += \
./fatfs/diskio.o    \
./fatfs/ff.o        \
./fatfs/ffsystem.o


C_DEPS += \
./fatfs/diskio.d \
./fatfs/ff.d        \
./fatfs/ffsystem.d

# Each subdirectory must supply rules for building sources it contributes
fatfs/%.o: ../fatfs/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../include -I../FreeRTOS_Kernel/include -I../fatfs/src \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


