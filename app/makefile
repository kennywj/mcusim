#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build
LIB = $(ODIR)/libapp.a
INCS = -I./duktape -I../FreeRTOS_Posix -I../FreeRTOS_Posix/FreeRTOS_Kernel/include \
	-I../FreeRTOS_Posix -I../port -I../fatfs/src -I../fatfs/port -I../port -I../port/freertos	\
	-I../lwip/src/include -I../shell -I../include
VPATH = .:./duktape
SRCS = app.c tcpecho.c ping.c  cmdnet.c cmdduktape.c duktape.c cmdfs.c 

OBJS = $(patsubst %.c,$(ODIR)/%.o,$(SRCS))
.PHONY: clean depend

all: $(LIB)

$(LIB): $(OBJS)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"
   
$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@  

clean:
	rm -f $(OBJS) $(LIB) *.bak

depend: $(SRCS)
	makedepend $(INCS) $<
