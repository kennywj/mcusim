#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
PROG = ./blesim

LIB_PATH := -L./build
INCS = -I./include -I./sys -I./FreeRTOS_Posix -I./FreeRTOS_Posix/FreeRTOS_Kernel/include	\
	-I./lwip_port -I./FreeRTOS_Posix/Common_Demo/include/


MKDIR_P = mkdir -p
CC = gcc
LD = ld
AR = ar

CFLAGS := -Wall -g -Wno-pointer-sign -fPIC
CFLAGS += -DDEBUG

LDFLAGS :=

export CC LD AR CFLAGS LDFLAGS MKDIR_P

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

SUBDIRS = sys shell FreeRTOS_Posix ble
LIBS = -lshell -lble -lfreertos -lsys -lpthread -lm 


.PHONY: subdirs $(SUBDIRS) clean all

all: $(PROG)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

$(PROG): $(SRCS) subdirs
	$(CC) $(CFLAGS) $(INCS) $(LIB_PATH) -Wl,-Map=$(PROG).map $< -o $@ $(LIBS)
	@echo "    Generate Program $(notdir $(PROG)) from $^"

clean:
	rm -rf build/* $(PROG)
