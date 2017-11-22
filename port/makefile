#
# 'make depend' to automatically generate dependences
# 'make' to rebuild object codes
# 'make clean' remove object files
#
ODIR = ../build
LIB = $(ODIR)/liblwip.a
INCS =  -I./ -I./arch -I./freertos -I../lwip/src/include -I../include	-I../FreeRTOS_Posix \
	-I../FreeRTOS_Posix/FreeRTOS_Kernel/include

VPATH = .:./freertos:../lwip/src/api:../lwip/src/apps:../lwip/src/core:../lwip/src/core/ipv4:../lwip/src/core/ipv6	\
	:../lwip/src/netif:../lwip/src/netif/ppp:../lwip/src/netif/ppp/polarssl
	
SRCS = ethernetif.c network.c sys_arch.c \
	ethernet.c lowpan6.c slipif.c	\
	api_lib.c api_msg.c err.c if_api.c netbuf.c netdb.c netifapi.c sockets.c tcpip.c	\
	def.c dns.c inet_chksum.c init.c ip.c mem.c memp.c netif.c pbuf.c raw.c stats.c sys.c	\
	tcp.c tcp_in.c tcp_out.c timeouts.c udp.c	\
	autoip.c dhcp.c etharp.c icmp.c igmp.c ip4.c ip4_addr.c ip4_frag.c	\
	dhcp6.c ethip6.c icmp6.c inet6.c nd6.c ip6.c ip6_addr.c ip6_frag.c mld6.c	\
	auth.c ccp.c chap_ms.c chap-md5.c chap-new.c demand.c eap.c ecp.c eui64.c 	\
	fsm.c ipcp.c ipv6cp.c lcp.c magic.c mppe.c multilink.c ppp.c pppapi.c pppcrypt.c 	\
	pppoe.c pppol2tp.c pppos.c upap.c utils.c vj.c	\
	arc4.c des.c md4.c md5.c sha1.c
	
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
