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

#ifndef PPP_SUPPORT

#define NET_IF_NUM 1
#define IFNAME0 'e'
#define IFNAME1 '0'

struct netif xnetif[NET_IF_NUM]; /* network interface structure */
static ip_addr_t ipaddr, netmask, gw;

//
//  function: init_netifs
//      network interface initialize
//
void init_netifs(int idx)
{
    int idx=0;
    
    IP_ADDR4(&gw,      192,168,1,1);
    IP_ADDR4(&ipaddr,  192,168,1,100);
    IP_ADDR4(&netmask, 255,255,255,0);
    netif_add(&xnetif[idx], ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw), NULL, ethernetif_init, tcpip_input);
    netif_set_default(&xnetif[idx]);
    netif_set_up(&xnetif[idx]);

    printf("IPV4: Host at %s ", ip4addr_ntoa(netif_ip4_addr(&xnetif[idx])));
    printf("mask %s ", ip4addr_ntoa(netif_ip4_netmask(&xnetif[idx])));
    printf("gateway %s\n", ip4addr_ntoa(netif_ip4_gw(&xnetif[idx])));
}
#endif
// not define PPP_SUPPORT


//
//  function: tcpip_init_done
//      callback function when TCPIP initial down
//
static void
tcpip_init_done(void *arg)
{
  sys_sem_t *sem;
  sem = (sys_sem_t *)arg;
  printf("TCP/IP initialized.\n");
//#ifndef PPP_SUPPORT
//    init_netifs();
//#endif
  sys_sem_signal(sem);
}


//
//  function: LwIP_Init
//      
//
//struct netif * 
void LwIP_Init(void)
{
	sys_sem_t sem;
	
	if (sys_sem_new(&sem, 0) != ERR_OK) 
	{
        LWIP_ASSERT("Failed to create semaphore", 0);
    }
    tcpip_init(tcpip_init_done, &sem);
    sys_sem_wait(&sem);
//#ifdef PPP_SUPPORT    
//    return NULL;
//#else    
//    return &xnetif[0];
//#endif
}

//
// command net, display network related information
//
int netif_info(char *buf, int size)
{
    struct netif *netif;
    char ipaddr[16],netmask[16],gw[16], *cp=buf, *end = &buf[size];
	
	buf[0]='\0';
    for(netif = netif_list; netif != NULL; netif = netif->next) {
        
        strncpy(ipaddr,ipaddr_ntoa(&netif->ip_addr),16);
        strncpy(netmask,ipaddr_ntoa(&netif->netmask),16);
        strncpy(gw,ipaddr_ntoa(&netif->gw),16);
        cp += snprintf(cp, end-cp, "%c%c: ip %s, mask %s, gw %s\n",netif->name[0], netif->name[1],
            ipaddr, netmask, gw);
        //printf("%c%c: ip %s, mask %s, gw %s\n",netif->name[0], netif->name[1],
        //ipaddr_ntoa(&netif->ip_addr), ipaddr_ntoa(&netif->netmask), ipaddr_ntoa(&netif->gw));
    }
    return 0;
}
