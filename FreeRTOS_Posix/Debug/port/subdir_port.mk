################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../port/freertos/ethernetif.c \
../port/freertos/network.c \
../port/freertos/sys_arch.c

OBJS += \
./port/ethernetif.o \
./port/network.o    \
./port/sys_arch.o

C_DEPS += \
./port/ethernetif.d \
./port/network.d    \
./port/sys_arch.d


# Each subdirectory must supply rules for building sources it contributes
port/%.o: ../port/freertos/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../include -I../FreeRTOS_Kernel/include -I../port -I../lwip/src/include \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


