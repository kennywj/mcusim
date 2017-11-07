################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fatfs/port/ramdisk.c  \
../fatfs/port/fattime.c  \
../fatfs/port/fs_port.c  \
../fatfs/port/cmdfs.c

OBJS += \
./fatfs/ramdisk.o   \
./fatfs/fattime.o   \
./fatfs/fs_port.o  \
./fatfs/cmdfs.o

C_DEPS += \
./fatfs/ramdisk.d   \
./fatfs/fattime.d   \
./fatfs/fs_port.d   \
./fatfs/cmdfs.d

# Each subdirectory must supply rules for building sources it contributes
fatfs/%.o: ../fatfs/port/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../include -I../shell -I../port -I../FreeRTOS_Kernel/include -I../fatfs/src \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


