################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/ipv4/autoip.c \
../lwip/src/core/ipv4/dhcp.c \
../lwip/src/core/ipv4/etharp.c \
../lwip/src/core/ipv4/icmp.c \
../lwip/src/core/ipv4/igmp.c \
../lwip/src/core/ipv4/ip4.c \
../lwip/src/core/ipv4/ip4_addr.c \
../lwip/src/core/ipv4/ip4_frag.c

OBJS += \
./lwip/autoip.o \
./lwip/dhcp.o \
./lwip/etharp.o \
./lwip/icmp.o \
./lwip/igmp.o \
./lwip/ip4.o \
./lwip/ip4_addr.o \
./lwip/ip4_frag.o

C_DEPS += \
./lwip/autoip.d \
./lwip/dhcp.d \
./lwip/etharp.d \
./lwip/icmp.d \
./lwip/igmp.d \
./lwip/ip4.d \
./lwip/ip4_addr.d \
./lwip/ip4_frag.d

# Each subdirectory must supply rules for building sources it contributes
lwip/%.o: ../lwip/src/core/ipv4/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../FreeRTOS_Kernel/include -I../port -I../lwip/src/include \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


