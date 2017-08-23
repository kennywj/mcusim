################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/netif/ppp/auth.c    \
../lwip/src/netif/ppp/ccp.c    \
../lwip/src/netif/ppp/chap_ms.c    \
../lwip/src/netif/ppp/chap-md5.c    \
../lwip/src/netif/ppp/chap-new.c    \
../lwip/src/netif/ppp/demand.c    \
../lwip/src/netif/ppp/eap.c    \
../lwip/src/netif/ppp/ecp.c    \
../lwip/src/netif/ppp/eui64.c    \
../lwip/src/netif/ppp/fsm.c    \
../lwip/src/netif/ppp/ipcp.c    \
../lwip/src/netif/ppp/ipv6cp.c    \
../lwip/src/netif/ppp/lcp.c    \
../lwip/src/netif/ppp/magic.c    \
../lwip/src/netif/ppp/mppe.c    \
../lwip/src/netif/ppp/multilink.c    \
../lwip/src/netif/ppp/ppp.c    \
../lwip/src/netif/ppp/pppapi.c    \
../lwip/src/netif/ppp/pppcrypt.c    \
../lwip/src/netif/ppp/pppoe.c    \
../lwip/src/netif/ppp/pppol2tp.c    \
../lwip/src/netif/ppp/pppos.c    \
../lwip/src/netif/ppp/upap.c    \
../lwip/src/netif/ppp/utils.c    \
../lwip/src/netif/ppp/vj.c

OBJS += \
./lwip/auth.o   \
./lwip/ccp.o   \
./lwip/chap_ms.o   \
./lwip/chap-md5.o   \
./lwip/chap-new.o   \
./lwip/demand.o   \
./lwip/eap.o   \
./lwip/ecp.o   \
./lwip/eui64.o   \
./lwip/fsm.o   \
./lwip/ipcp.o   \
./lwip/ipv6cp.o   \
./lwip/lcp.o   \
./lwip/magic.o   \
./lwip/mppe.o   \
./lwip/multilink.o   \
./lwip/ppp.o   \
./lwip/pppapi.o   \
./lwip/pppcrypt.o   \
./lwip/pppoe.o   \
./lwip/pppol2tp.o   \
./lwip/pppos.o   \
./lwip/upap.o   \
./lwip/utils.o   \
./lwip/vj.o


C_DEPS += \
./lwip/auth.d   \
./lwip/ccp.d   \
./lwip/chap_ms.d   \
./lwip/chap-md5.d   \
./lwip/chap-new.d   \
./lwip/demand.d   \
./lwip/eap.d   \
./lwip/ecp.d   \
./lwip/eui64.d   \
./lwip/fsm.d   \
./lwip/ipcp.d   \
./lwip/ipv6cp.d   \
./lwip/lcp.d   \
./lwip/magic.d   \
./lwip/mppe.d   \
./lwip/multilink.d   \
./lwip/ppp.d   \
./lwip/pppapi.d   \
./lwip/pppcrypt.d   \
./lwip/pppoe.d   \
./lwip/pppol2tp.d   \
./lwip/pppos.d   \
./lwip/upap.d   \
./lwip/utils.d   \
./lwip/vj.d


# Each subdirectory must supply rules for building sources it contributes
lwip/%.o: ../lwip/src/netif/ppp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../. -I../FreeRTOS_Kernel/include -I../port -I../lwip/src/include \
	-O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


