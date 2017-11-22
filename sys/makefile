#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build
LIB = $(ODIR)/sys.a
INCS = -I./duktape -I../include -I../FreeRTOS_Posix  -I../FreeRTOS_Posix/FreeRTOS_Kernel/include \
	-I../lwip/src/include -I../shell -I../port -I../port/freertos
VPATH = ./
SRCS = crc16.c ping.c sys.c

OBJS = $(patsubst %.c,$(ODIR)/%.o,$(SRCS))
.PHONY: clean depend all

all: $(LIB)

$(LIB): $(OBJS)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"
        
$(ODIR)/%.o: %.c
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@  

clean:
	rm -f $(OBJS)

depend: $(SRCS)
	makedepend $(INCS) $<
