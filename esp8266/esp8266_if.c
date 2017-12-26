/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include <string.h>
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/timeouts.h"
#include "lwip/sys.h"
#include "netif/etharp.h"
#include "lwip/err.h"
#include "arch/sys_arch.h"

#include "netconf.h"
#include "esp8266_if.h"
#include "comm.h"


#define HSUART_CH_NETWORK	1
#define COMMAND_DELAY		100 //ms

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 'o'


static struct netif *s_pxNetIf;
extern int netif_use_dhcp;
extern int netif_dhcp_task_exist;
static unsigned int network_state;

#define NETM_LINKED			0x1
//#define NETM_IP_ASSIGNED	0x2

//static void esp8266_arp_timer(void *arg);

struct msg_station_conf test_ap = {
	sizeof(AP_SSID),
	AP_SSID,
	sizeof(AP_PASSWORD),
	AP_PASSWORD,
};

// Private function prototypes

/*void uart_tx_one_char(uint8_t tx_char)
{
	hsuart_send(HSUART_CH_NETWORK, &tx_char, 1);
}*/

/*extern void HSUART_rx_timeout_set(uint8_t ch, uint32_t value);
uint8_t uart_recv_one_char(void)
{
	uint8_t rx_char;

	HSUART_rx_timeout_set(HSUART_CH_NETWORK, 0xFFFFFFFF);
	hsuart_receive(HSUART_CH_NETWORK, &rx_char, 1);

	return rx_char;
}*/

void esp8266_connect(void)
{
	printf("%s,%d\n", __FUNCTION__, __LINE__);

	comm_send_begin(MSG_LOG_LEVEL_SET);
	comm_send_u8(10);
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);

	comm_send_begin(MSG_SET_FORWARDING_MODE);
	comm_send_u8(FORWARDING_MODE_ETHER);
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);

	comm_send_begin(MSG_WIFI_SLEEP_MODE_SET);
	comm_send_u8(WIFI_SLEEP_NONE);
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);

	comm_send_begin(MSG_WIFI_MODE_SET);
	comm_send_u8(MODE_STA);
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);

	printf("trying to get mac address...\n");
	comm_send_begin(MSG_WIFI_GET_MACADDR_REQUEST);
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);

	printf("trying to connect AP...\n");
	comm_send_begin(MSG_STATION_CONF_SET);
	comm_send_data((uint8_t *) &test_ap, sizeof(struct msg_station_conf));
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);

#if 0

	printf("set to dhcp client mode...\n");
	comm_send_begin(MSG_STATION_DHCPC_STATE_SET);
	comm_send_u8(1);
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);

	comm_send_begin(MSG_STATION_IP_CONF_REQUEST);
	comm_send_end();
	vTaskDelay(COMMAND_DELAY);
#endif
}

static void
packet_from_device(uint8_t type, uint8_t *data, uint32_t n)
{
	uint32_t l=0;
	struct pbuf *p, *q;
	struct msg_ip_conf *ip_conf;
	//printf("packet is coming...\n");

	switch(type) {
	case MSG_ETHER_PACKET:
		//printf("receive raw packet...\n");
		/* We allocate a pbuf chain of pbufs from the pool. */
		p = pbuf_alloc(PBUF_RAW, n, PBUF_POOL);
 
		/* Copy received frame from ethernet driver buffer to stack buffer */
		if (p) { 
			for (q = p; q != NULL; q = q->next) {
				memcpy((u8_t *)q->payload, (u8_t *)(data+l), q->len);
				l = l + q->len;
			}
			if (ERR_OK != s_pxNetIf->input(p, s_pxNetIf)) {
				pbuf_free(p);
		        p = NULL;
			}
		} else {
			printf("can't alloc memory to contain raw packet...\n");
		}
		break;
	case MSG_WIFI_GET_MACADDR_REPLY:
		printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", data[0], data[1], data[2], data[3], data[4], data[5]);
		s_pxNetIf->hwaddr[0] =  data[0];
		s_pxNetIf->hwaddr[1] =  data[1];
		s_pxNetIf->hwaddr[2] =  data[2];
		s_pxNetIf->hwaddr[3] =  data[3];
		s_pxNetIf->hwaddr[4] =  data[4];
		s_pxNetIf->hwaddr[5] =  data[5];
		break;
	case MSG_STATION_IP_CONF_REPLY:
		ip_conf = (struct msg_ip_conf *) data;
		printf("sizeof(msg_ip_conf) = %d\n", sizeof(struct msg_ip_conf));
		printf("receive size n = %ld\n", n);

		printf("address: 0x%lx\n", ip_conf->address);
		printf("netmask: 0x%lx\n", ip_conf->netmask);
		printf("gateway: 0x%lx\n", ip_conf->gateway);
		printf("dns[0]: 0x%lx\n", ip_conf->dns[0]);
		printf("dns[1]: 0x%lx\n", ip_conf->dns[1]);
		printf("dns[2]: 0x%lx\n", ip_conf->dns[2]);
		 
		break;
	case MSG_STATUS:
		if (n) {
			printf("MSG_STATUS: 0x%x\n", data[0]);
		} else {
			printf("Wrong size of MSG_STATUS payload: %ld\n", n);
		}
		break;
	case MSG_STATION_CONN_STATUS_REPLY:
		if (n) {
			//printf("MSG_STATION_CONN_STATUS_REPLY: %d\n", data[0]);
			switch (data[0]) {
			case 1: //connecting
			case 5: //got ip
				if ((network_state & NETM_LINKED) == 0) {
					network_state |= NETM_LINKED;
					printf("link up\n");
					netif_set_link_up(s_pxNetIf); 
					if (netif_use_dhcp) {
						/* Start DHCPClient */
						if (netif_dhcp_task_exist == 0)
							xTaskCreate(LwIP_DHCP_task, "dhcp_c", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
					}
				}
				break;
			default:
				network_state &= ~NETM_LINKED;
				printf("link down\n");
				netif_set_link_down(s_pxNetIf); 
				break;
			}
		} else {
			printf("Wrong size of MSG_STATION_CONN_STATUS_REPLY payload: %ld\n", n);
		}
		break;
	case MSG_BOOT:
		printf("esp8266 is rebooting, reset network...\n");
		if (network_state & NETM_LINKED)
			netif_set_link_down(s_pxNetIf);
		network_state &= ~NETM_LINKED;
		vTaskDelay(2000); //wait for esp8266 boot up
		break;
	case MSG_LOG:
/*
		if (n >= 3) {
			printf("MSG_LOG:\n type: %d\n", data[0]);
			printf("message: %s\n", &data[1]);
		} else {
			printf("Wrong size of MSG_LOG payload: %ld\n", n);
		}
*/
		break;
	default:
		printf("packet type: 0x%x doesn't have handle code!!!\n", type);
		break;
	}
}


//
// link manager task
//
void tsk_network_mngr(void * pvParameters)
{
	int wait_dhcp_cnt = 0;
	extern int esp8266_exit;
	
	vTaskDelay(2000); //wait for esp8266 boot up
	while (1) {
		if (esp8266_exit)
			break;
		if ((wait_dhcp_cnt == 0) && (network_state & NETM_LINKED) == 0) {
			esp8266_connect();
			vTaskDelay(3000);
			wait_dhcp_cnt = 30;
		}
		comm_send_begin(MSG_STATION_CONN_STATUS_REQUEST);
		comm_send_end();
		vTaskDelay(1000);
		if (wait_dhcp_cnt)
			wait_dhcp_cnt--;
	}
	// end rx task
	vTaskDelete( NULL );
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void esp8266_low_level_init(struct netif *netif)
{
	/* set netif MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set netif MAC hardware address */
	netif->hwaddr[0] =  MAC_ADDR0;
	netif->hwaddr[1] =  MAC_ADDR1;
	netif->hwaddr[2] =  MAC_ADDR2;
	netif->hwaddr[3] =  MAC_ADDR3;
	netif->hwaddr[4] =  MAC_ADDR4;
	netif->hwaddr[5] =  MAC_ADDR5;

	/* set netif maximum transfer unit */
	netif->mtu = 1500;

	/* Accept broadcast address and ARP traffic */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

	s_pxNetIf =netif;

	//hsuart_init(HSUART_CH_NETWORK);
	comm_init(packet_from_device);

	xTaskCreate(tsk_uart_reader, "urt_prsr", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(tsk_network_mngr, "net_mngr", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
	/* Start DHCPClient */
	//if (netif_use_dhcp)
	//	xTaskCreate(LwIP_DHCP_task, "dhcp_c", configMINIMAL_STACK_SIZE * 2, NULL, tskNORMAL_PRIORITY, NULL);

}


/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t esp8266_low_level_output(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
  
	comm_send_begin(MSG_ETHER_PACKET);
    for(q = p; q != NULL; q = q->next) 
    {
		comm_send_data(q->payload, q->len);
    }
	comm_send_end();

	return ERR_OK;
}

      
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t esp8266_if_init(struct netif *netif)
{
	LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;

	netif->output = etharp_output;
	netif->linkoutput = esp8266_low_level_output;

	/* initialize the hardware */
	esp8266_low_level_init(netif);
	
	return ERR_OK;
}

