#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build
LIB = $(ODIR)/libfreertos.a
INCS =  -I./ -I./AsyncIO -I./FreeRTOS_Kernel/include -I../port  \
	-I./Common_Demo/include
VPATH = .:./AsyncIO:./FileIO:./FreeRTOS_Kernel:./FreeRTOS_Kernel/portable/GCC/Posix:	\
	./FreeRTOS_Kernel/portable/MemMang:./Common_Demo/Minimal:./ParTest
	
SRCS = fileIO.c AsyncIO.c AsyncIOSerial.c AsyncIOSocket.c PosixMessageQueueIPC.c	\
	croutine.c list.c queue.c tasks.c port.c heap_3.c	\
	blocktim.c countsem.c crflash.c crhook.c GenQTest.c QPeek.c recmutex.c  \
	ParTest.c
	
OBJS = $(patsubst %.c,$(ODIR)/%.o,$(SRCS))

.PHONY: clean depend

all: $(LIB)

$(LIB): $(OBJS)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"

$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@  

clean:
	rm -f $(OBJS)

depend: $(SRCS)
	makedepend $(INCS) $<
