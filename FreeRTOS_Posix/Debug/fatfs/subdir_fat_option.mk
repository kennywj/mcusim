################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fatfs/src/option/syscall.c   \
../fatfs/src/option/cc950.c

OBJS += \
./fatfs/syscall.o   \
./fatfs/cc950.o


C_DEPS += \
./fatfs/syscall.d   \
./fatfs/cc950.d

# Each subdirectory must supply rules for building sources it contributes
fatfs/%.o: ../fatfs/src/option/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../include -I../FreeRTOS_Kernel/include -I../fatfs/src \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


