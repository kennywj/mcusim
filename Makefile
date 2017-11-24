#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
PROG = ./u2w

LIB_PATH := -L./build
INCS = -I./include -I./sys -I./FreeRTOS_Posix -I./FreeRTOS_Posix/FreeRTOS_Kernel/include	\
	-I./port -I./FreeRTOS_Posix/Common_Demo/include/
#	
# p.s the link order of library has depenendce, not change
#
LIBS = -lshell -lapp -lsys -llwip -lfreertos -lfatfs -lpthread -lm

CC = gcc
LD = ld
AR = ar

CFLAGS := -Wall -g
CFLAGS += -DDEBUG

LDFLAGS :=

export CC LD AR CFLAGS LDFLAGS

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

SUBDIRS = app sys shell FreeRTOS_Posix fatfs port

.PHONY: subdirs $(SUBDIRS) clean all

all: $(PROG)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

$(PROG): $(SRCS) subdirs 
	$(CC) $(CFLAGS) $(INCS) $(LIB_PATH) -Wl,-Map=$(PROG).map $< -o $@ $(LIBS)
	@echo "    Generate Program $(notdir $(PROG)) from $^"	
    
clean:
	rm -rf *.o build/*.o build/*.a
