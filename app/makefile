#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build
LIB = $(ODIR)/libapp.a
INCS = -I./duktape -I./http-parser -I./gnss -I./json -I../FreeRTOS_Posix -I../FreeRTOS_Posix/FreeRTOS_Kernel/include \
	-I../FreeRTOS_Posix -I../port -I../fatfs/src -I../fatfs/port -I../port -I../port/freertos	\
	-I../mbedtls-2.6.0/include/mbedtls -I../lwip/src/include -I../shell -I../include
VPATH = .:./duktape:./http-parser:./http-parser/contrib:./gnss:./json
SRCS = app.c tcpecho.c ping.c  cmdnet.c cmdduktape.c duktape.c cmdfs.c http_parser.c url_parser.c \
	parsertrace.c http_test.c tftps.c https.c pppd.c xmodem.c camera.c cmdmd5.c gnss.c minmea.c allystar.c	\
	tiny-json.c json-maker.c cmd_json.c

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
