#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
PROG = ./u2w

LIB_PATH := -L./build
INCS = -I./include -I./sys -I./FreeRTOS_Posix -I./FreeRTOS_Posix/FreeRTOS_Kernel/include	\
	-I./port -I./FreeRTOS_Posix/Common_Demo/include/
LIBS := $(LIB_PATH)/sys.a $(LIB_PATH)/shell.a $(LIB_PATH)/freertos.a $(LIB_PATH)/lwip.a \
	$(LIB_PATH)/fatfs.a

CC = gcc
LD = ld
AR = ar

CFLAGS := -Wall -g
CFLAGS += -DDEBUG

LDFLAGS :=

export CC LD AR CFLAGS LDFLAGS

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

SUBDIRS = sys shell FreeRTOS_Posix fatfs port

.PHONY: subdirs $(SUBDIRS) clean all

all: subdirs $(PROG)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
	
$(PROG): $(SRCS)
	$(CC) $(CFLAGS) $(INCS) -Wl,-Map=$(PROG).map $(LIBS) $^ -o $@ -lm -lpthread
	@echo "    Generate Program $(notdir $(PROG)) from $^"	
    
clean:
	rm -rf *.o build/*.o build/*.a
