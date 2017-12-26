#ifndef __NETCONF_H__
#define __NETCONF_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>

/* MAC ADDRESS*/
#define MAC_ADDR0       0x02
#define MAC_ADDR1       0x00
#define MAC_ADDR2       0x00
#define MAC_ADDR3       0x00
#define MAC_ADDR4       0x00
#define MAC_ADDR5       0x00

/*Static IP ADDRESS*/
#define IP_ADDR0        192
#define IP_ADDR1        168
#define IP_ADDR2        98
#define IP_ADDR3        100

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0        192
#define GW_ADDR1        168
#define GW_ADDR2        98
#define GW_ADDR3        1

/*Wifi SSID*/
#define AP_SSID         "kennyhd"
#define AP_PASSWORD     "wj706101"

void esp8266_netinit(int use_dhcp);
int esp8266_input(void);
int esp8266_output(unsigned char *buf, unsigned int len);
void LwIP_DHCP_task(void * pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H__ */

