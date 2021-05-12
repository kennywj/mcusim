
message("CURRENT_DIR= ${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/lwip/src/include
	${CMAKE_SOURCE_DIR}/mbedtls-2.6.0/include/mbedtls
	${CMAKE_CURRENT_SOURCE_DIR}
)
# Create a library called "port" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library (lwip STATIC 
	freertos/network.c 
	freertos/sys_arch.c 
	freertos/ethernetif.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ethernet.c 
	${CMAKE_SOURCE_DIR}/lwip/src/netif/lowpan6.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/slipif.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/api_lib.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/api_msg.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/err.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/if_api.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/netbuf.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/netdb.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/netifapi.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/sockets.c
	${CMAKE_SOURCE_DIR}/lwip/src/api/tcpip.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/def.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/dns.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/inet_chksum.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/init.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ip.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/mem.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/memp.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/netif.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/pbuf.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/raw.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/stats.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/sys.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/tcp.c 
	${CMAKE_SOURCE_DIR}/lwip/src/core/tcp_in.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/tcp_out.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/timeouts.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/udp.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/autoip.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/dhcp.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/etharp.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/icmp.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/igmp.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/ip4.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/ip4_addr.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv4/ip4_frag.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/dhcp6.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/ethip6.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/icmp6.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/inet6.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/nd6.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/ip6.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/ip6_addr.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/ip6_frag.c
	${CMAKE_SOURCE_DIR}/lwip/src/core/ipv6/mld6.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/auth.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/ccp.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/chap_ms.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/chap-md5.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/chap-new.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/demand.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/eap.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/ecp.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/eui64.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/fsm.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/ipcp.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/ipv6cp.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/lcp.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/magic.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/mppe.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/multilink.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/ppp.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/pppapi.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/pppcrypt.c
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/pppoe.c 
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/pppol2tp.c 
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/pppos.c 
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/upap.c 
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/utils.c 
	${CMAKE_SOURCE_DIR}/lwip/src/netif/ppp/vj.c)
	
# Make sure the compiler can find include files for our app library
# when other libraries or executables link to app
target_include_directories(lwip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})