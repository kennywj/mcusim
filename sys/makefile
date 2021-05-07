#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build/sys
LIB = ../build/libsys.a
INCS = -I./duktape -I../include -I../FreeRTOS_Posix  -I../FreeRTOS_Posix/FreeRTOS_Kernel/include \
	-I../mbedtls-2.6.0/include/mbedtls -I../lwip/src/include -I../shell -I../lwip_port -I../lwip_port/freertos
VPATH = ./
SRCS = sys.c util.c fifo.c cmd_fifo.c

OBJS = $(patsubst %.c,$(ODIR)/%.o,$(SRCS))

.PHONY: directories clean depend all

all: directories $(LIB)

directories: $(ODIR)

$(ODIR):
	$(MKDIR_P) $(ODIR)
	
$(LIB): $(OBJS)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"

$(ODIR)/%.o: %.c
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB) *.bak

depend: $(SRCS)
	makedepend $(INCS) $<
