#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build/coremark
LIB = ../build/libcoremark.a
INCS =  -I./ -I../coremark -I../include	-I../FreeRTOS_Posix \
	-I../FreeRTOS_Posix/FreeRTOS_Kernel/include 

VPATH = .:../coremark

ITERATIONS = 10

#sinclude core_portme.mk
	
SRCS = core_list_join.c core_main.c core_matrix.c core_state.c core_util.c core_portme.c
	
OBJS = $(patsubst %.c,$(ODIR)/%.o,$(SRCS))

.PHONY: directories clean depend

all: directories $(LIB)

directories: $(ODIR)

$(ODIR):
	$(MKDIR_P) $(ODIR)
	
$(LIB): $(OBJS)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"

$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@  

clean:
	rm -f $(OBJS)

depend: $(SRCS)
	makedepend $(INCS) $<
