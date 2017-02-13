################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/def.c \
../lwip/src/core/dns.c \
../lwip/src/core/inet_chksum.c \
../lwip/src/core/init.c \
../lwip/src/core/ip.c \
../lwip/src/core/mem.c \
../lwip/src/core/memp.c \
../lwip/src/core/netif.c \
../lwip/src/core/pbuf.c \
../lwip/src/core/raw.c \
../lwip/src/core/stats.c \
../lwip/src/core/sys.c \
../lwip/src/core/tcp.c \
../lwip/src/core/tcp_in.c \
../lwip/src/core/tcp_out.c \
../lwip/src/core/timeouts.c \
../lwip/src/core/udp.c

OBJS += \
./lwip/def.o \
./lwip/dns.o \
./lwip/inet_chksum.o \
./lwip/init.o \
./lwip/ip.o \
./lwip/mem.o \
./lwip/memp.o \
./lwip/netif.o \
./lwip/pbuf.o \
./lwip/raw.o \
./lwip/stats.o \
./lwip/sys.o \
./lwip/tcp.o \
./lwip/tcp_in.o \
./lwip/tcp_out.o \
./lwip/timeouts.o \
./lwip/udp.o

C_DEPS += \
./lwip/def.d \
./lwip/dns.d \
./lwip/inet_chksum.d \
./lwip/init.d \
./lwip/ip.d \
./lwip/mem.d \
./lwip/memp.d \
./lwip/netif.d \
./lwip/pbuf.d \
./lwip/raw.d \
./lwip/stats.d \
./lwip/sys.d \
./lwip/tcp.d \
./lwip/tcp_in.d \
./lwip/tcp_out.d \
./lwip/timeouts.d \
./lwip/udp.d


# Each subdirectory must supply rules for building sources it contributes
lwip/%.o: ../lwip/src/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../FreeRTOS_Kernel/include -I../port -I../lwip/src/include \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


