################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../uart2wifi/uart2wifi.c    \
../uart2wifi/crc16.c    \
../uart2wifi/ping.c

OBJS += \
./uart2wifi/uart2wifi.o \
./uart2wifi/crc16.o \
./uart2wifi/ping.o

C_DEPS += \
./uart2wifi/uart2wifi.d \
./uart2wifi/crc16.d \
./uart2wifi/ping.d


# Each subdirectory must supply rules for building sources it contributes
uart2wifi/%.o: ../uart2wifi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix \
	-I../include -I../port -I../port/freertos -I../lwip/src/include -I../shell -O0 -g -Wall -c -fmessage-length=0 \
	-pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


