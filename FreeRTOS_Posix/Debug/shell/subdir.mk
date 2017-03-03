################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../shell/cmd.c \
../shell/console.c \
../shell/parser.c \
../shell/util.c \
../shell/duktape/duktape.c

OBJS += \
./shell/cmd.o \
./shell/console.o \
./shell/parser.o \
./shell/util.o  \
./shell/duktape/duktape.o

C_DEPS += \
./shell/cmd.d \
./shell/console.d \
./shell/parser.d \
./shell/util.d  \
./shell/duktape/duktape.d


# Each subdirectory must supply rules for building sources it contributes
shell/%.o: ../shell/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../shell/duktape -I../FreeRTOS_Kernel/include \
	-I../FreeRTOS_Kernel/portable/GCC/Posix -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


