################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/api/api_lib.c \
../lwip/src/api/api_msg.c \
../lwip/src/api/err.c \
../lwip/src/api/if_api.c \
../lwip/src/api/netbuf.c \
../lwip/src/api/netdb.c \
../lwip/src/api/netifapi.c \
../lwip/src/api/sockets.c \
../lwip/src/api/tcpip.c

OBJS += \
./lwip/api_lib.o \
./lwip/api_msg.o \
./lwip/err.o \
./lwip/if_api.o \
./lwip/netbuf.o \
./lwip/netdb.o \
./lwip/netifapi.o \
./lwip/sockets.o \
./lwip/tcpip.o

C_DEPS += \
./lwip/api_lib.d \
./lwip/api_msg.d \
./lwip/err.d \
./lwip/if_api.d \
./lwip/netbuf.d \
./lwip/netdb.d \
./lwip/netifapi.d \
./lwip/sockets.d \
./lwip/tcpip.d


# Each subdirectory must supply rules for building sources it contributes
lwip/%.o: ../lwip/src/api/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../FreeRTOS_Kernel/include -I../port -I../lwip/src/include \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


