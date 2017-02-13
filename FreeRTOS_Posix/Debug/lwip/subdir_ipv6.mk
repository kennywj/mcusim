################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/ipv6/dhcp6.c \
../lwip/src/core/ipv6/ethip6.c \
../lwip/src/core/ipv6/icmp6.c \
../lwip/src/core/ipv6/inet6.c \
../lwip/src/core/ipv6/nd6.c \
../lwip/src/core/ipv6/ip6.c \
../lwip/src/core/ipv6/ip6_addr.c \
../lwip/src/core/ipv6/ip6_frag.c \
../lwip/src/core/ipv6/mld6.c


OBJS += \
./lwip/dhcp6.o \
./lwip/ethip6.o \
./lwip/icmp6.o \
./lwip/inet6.o \
./lwip/nd6.o \
./lwip/ip6.o \
./lwip/ip6_addr.o \
./lwip/ip6_frag.o \
./lwip/mld6.o

C_DEPS += \
./lwip/dhcp6.d \
./lwip/ethip6.d \
./lwip/icmp6.d \
./lwip/inet6.d \
./lwip/nd6.d \
./lwip/ip6.d \
./lwip/ip6_addr.d \
./lwip/ip6_frag.d \
./lwip/mld6.d

# Each subdirectory must supply rules for building sources it contributes
lwip/%.o: ../lwip/src/core/ipv6/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../FreeRTOS_Kernel/include -I../port -I../lwip/src/include \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


