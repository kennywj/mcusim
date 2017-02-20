#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

int ethernetif_recv(struct netif *netif, char *data, int total_len);
err_t ethernetif_init(struct netif *netif);
void lwip_PRE_SLEEP_PROCESSING(void);
void lwip_POST_SLEEP_PROCESSING(void);

#endif 
