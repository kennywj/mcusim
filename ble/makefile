#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build/ble
LIB = ../build/libble.a
INCS = -I./ -I../include -I../FreeRTOS_Posix  -I../FreeRTOS_Posix/FreeRTOS_Kernel/include -I../shell -I../sys
VPATH = ./

SRCS = vphy.c cmdble.c

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
