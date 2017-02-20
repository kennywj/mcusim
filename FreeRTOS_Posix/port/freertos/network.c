//
// porting Lwip
//

#include "lwip/init.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
#include "lwip/debug.h"

#include "ethernetif.h"
#include "FreeRTOS.h"
#include "task.h"


#define NET_IF_NUM 1
#define IFNAME0 'e'
#define IFNAME1 '0'

struct netif xnetif[NET_IF_NUM]; /* network interface structure */
static ip_addr_t ipaddr, netmask, gw;

unsigned char debug_flags = (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT);

//
//  function: init_netifs
//      network interface initialize
//
static void
init_netifs(void)
{
    int idx=0;
    
    IP_ADDR4(&gw,      192,168,1,1);
    IP_ADDR4(&ipaddr,  192,168,1,200);
    IP_ADDR4(&netmask, 255,255,255,0);
    netif_add(&xnetif[idx], ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw), NULL, ethernetif_init, tcpip_input);
    netif_set_default(&xnetif[idx]);
    netif_set_up(&xnetif[idx]);

    printf("IPV4: Host at %s ", ip4addr_ntoa(netif_ip4_addr(&xnetif[idx])));
    printf("mask %s ", ip4addr_ntoa(netif_ip4_netmask(&xnetif[idx])));
    printf("gateway %s\n", ip4addr_ntoa(netif_ip4_gw(&xnetif[idx])));
}

//
//  function: tcpip_init_done
//      callback function when TCPIP initial down
//
static void
tcpip_init_done(void *arg)
{
  sys_sem_t *sem;
  sem = (sys_sem_t *)arg;

  init_netifs();

  sys_sem_signal(sem);
}


//
//  function: LwIP_Init
//      
//
struct netif * 
LwIP_Init(void)
{
	sys_sem_t sem;
	
	if (sys_sem_new(&sem, 0) != ERR_OK) 
	{
        LWIP_ASSERT("Failed to create semaphore", 0);
    }
    tcpip_init(tcpip_init_done, &sem);
    sys_sem_wait(&sem);
    
    printf("TCP/IP initialized.\n");
    return &xnetif[0];
}