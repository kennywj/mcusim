#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build
LIB = $(ODIR)/libesp8266.a
INCS = -I./ -I../include -I../FreeRTOS_Posix  -I../FreeRTOS_Posix/FreeRTOS_Kernel/include \
	-I../lwip/src/include -I../shell -I../port -I../port/freertos \
	-I../mbedtls-2.6.0/include/mbedtls
VPATH = ./
SRCS = cobs.c comm.c crc16.c esp8266_if.c netconf.c

OBJS = $(patsubst %.c,$(ODIR)/%.o,$(SRCS))
.PHONY: clean depend all

all: $(LIB)

$(LIB): $(OBJS)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"
        
$(ODIR)/%.o: %.c
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@  

clean:
	rm -f $(OBJS) $(LIB) *.bak

depend: $(SRCS)
	makedepend $(INCS) $<
