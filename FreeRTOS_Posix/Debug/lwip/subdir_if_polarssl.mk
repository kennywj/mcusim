################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/netif/ppp/polarssl/arc4.c  \
../lwip/src/netif/ppp/polarssl/des.c  \
../lwip/src/netif/ppp/polarssl/md4.c  \
../lwip/src/netif/ppp/polarssl/md5.c  \
../lwip/src/netif/ppp/polarssl/sha1.c

OBJS += \
./lwip/arc4.o  \
./lwip/des.o  \
./lwip/md4.o  \
./lwip/md5.o  \
./lwip/sha1.o


C_DEPS += \
./lwip/arc4.d  \
./lwip/des.d  \
./lwip/md4.d  \
./lwip/md5.d  \
./lwip/sha1.d


# Each subdirectory must supply rules for building sources it contributes
lwip/%.o: ../lwip/src/netif/ppp/polarssl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../FreeRTOS_Kernel/include -I../port -I../lwip/src/include \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


