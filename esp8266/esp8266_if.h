#ifndef __ESP8266_IF_H__
#define __ESP8266_IF_H__

#include "lwip/err.h"
#include "lwip/netif.h"

//void uart_tx_one_char(uint8_t tx_char);
//uint8_t uart_recv_one_char(void);
int esp8266_output(unsigned char *buf, unsigned int len);
int esp8266_input(void);

err_t esp8266_if_init(struct netif *netif);

#endif /* __ESP8266_IF_H__ */

