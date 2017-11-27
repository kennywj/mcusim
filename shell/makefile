#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build
LIB = $(ODIR)/libshell.a
INCS = -I../include -I../FreeRTOS_Posix -I../FreeRTOS_Posix/FreeRTOS_Kernel/include \
	-I../FreeRTOS_Posix -I../port -I../fatfs/src -I../fatfs/port -I./duktape 
VPATH = .:./duktape
SRCS = cmd.c console.c crc16.c parser.c util.c duktape.c cmdfs.c

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
# DO NOT DELETE

cmd.o: /usr/include/ctype.h /usr/include/features.h
cmd.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
cmd.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
cmd.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
cmd.o: /usr/include/endian.h /usr/include/bits/endian.h
cmd.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
cmd.o: /usr/include/xlocale.h /usr/include/stdio.h /usr/include/libio.h
cmd.o: /usr/include/_G_config.h /usr/include/wchar.h
cmd.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
cmd.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
cmd.o: /usr/include/bits/waitstatus.h /usr/include/sys/types.h
cmd.o: /usr/include/time.h /usr/include/sys/select.h
cmd.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
cmd.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
cmd.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
cmd.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
cmd.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
cmd.o: /usr/include/bits/uio.h /usr/include/bits/socket.h
cmd.o: /usr/include/bits/socket_type.h /usr/include/bits/sockaddr.h
cmd.o: /usr/include/asm/socket.h /usr/include/asm-generic/socket.h
cmd.o: /usr/include/asm/sockios.h /usr/include/asm-generic/sockios.h
cmd.o: /usr/include/netinet/in.h /usr/include/stdint.h
cmd.o: /usr/include/bits/wchar.h /usr/include/bits/in.h
cmd.o: /usr/include/arpa/inet.h /usr/include/unistd.h
cmd.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
cmd.o: /usr/include/bits/confname.h /usr/include/getopt.h
cmd.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h
cmd.o: /usr/include/bits/setjmp.h cmd.h ./duktape/duktape.h
cmd.o: ./duktape/duk_config.h /usr/include/sys/time.h /usr/include/setjmp.h
cmd.o: /usr/include/math.h /usr/include/bits/math-vector.h
cmd.o: /usr/include/bits/libm-simd-decl-stubs.h /usr/include/bits/huge_val.h
cmd.o: /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h
cmd.o: /usr/include/bits/inf.h /usr/include/bits/nan.h
cmd.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
cmd.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
cmd.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
cmd.o: /usr/include/bits/posix2_lim.h
