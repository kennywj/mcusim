/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   Network connection configuration
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */
/**
  ******************************************************************************
  * <h2><center>&copy; Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.</center></h2>
  * @file    netconf.c
  * @author  CMP Team
  * @version V1.0.0
  * @date    28-December-2012
  * @brief   Network connection configuration     
  *          Modified to support the STM32F4DISCOVERY, STM32F4DIS-BB and
  *          STM32F4DIS-LCD modules. 
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, Embest SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
  * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
  * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
  * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "netconf.h"
#include "esp8266_if.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSerial.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "sys.h"
#include "cmd.h"


/* Private typedef -----------------------------------------------------------*/
typedef enum 
{ 
  DHCP_START=0,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT
} 
DHCP_State_TypeDef;
/* Private define ------------------------------------------------------------*/
#define MAX_DHCP_TRIES 5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif xnetif; /* network interface structure */
int netif_use_dhcp;
int netif_dhcp_task_exist;

//
// setup serial port structure
//
int esp8266_exit = 0;
static int esp8266_on=0;
static xQueueHandle xSerialRxQueue;
static char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
static int iSerialReceive = 0;
static xTaskHandle hSerialTask;
static int baudid=3;	// default 115200
xSemaphoreHandle esp8266_sem =	NULL;



/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the lwip network interface
  * @param  use_dhsp control flag
  * @retval None
  */
void esp8266_netinit(int use_dhcp)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

  netif_use_dhcp = use_dhcp;
  /* IP address setting & display on STM32_evalboard LCD*/
  if (use_dhcp) {
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;
  } else {
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
  }

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))
    
   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr_t structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/

  netif_add(&xnetif, &ipaddr, &netmask, &gw, NULL, &esp8266_if_init, &tcpip_input);

 /*  Registers the default network interface. */
  netif_set_default(&xnetif);

 /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&xnetif);
  //netif_set_link_up(&xnetif); 
}

static void print_ip_info(ip_addr_t addr)
{
  char buffer[32];

  ipaddr_ntoa_r(&addr, buffer, 32);
  printf("%s\n", buffer);
}

/**
  * @brief  LwIP_DHCP_Process_Handle
  * @param  None
  * @retval None
  */
void LwIP_DHCP_task(void * pvParameters)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  uint8_t DHCP_state;
  struct dhcp *dhcp = netif_dhcp_data(&xnetif);

  DHCP_state = DHCP_START;

  netif_dhcp_task_exist = 1;
  vTaskDelay(3000);
  printf("DHCP task start...\n");
  for (;;)
  {
    switch (DHCP_state)
    {
      case DHCP_START:
      {
        dhcp_start(&xnetif);
        DHCP_state = DHCP_WAIT_ADDRESS;
      }
      break;

      case DHCP_WAIT_ADDRESS:
      {
        if (dhcp_supplied_address(&xnetif)) {
          DHCP_state = DHCP_ADDRESS_ASSIGNED;	
          
          /* Stop DHCP */
          dhcp_stop(&xnetif);

          printf("dhcp got ip success\n");
          printf("ip address: ");
          print_ip_info(xnetif.ip_addr);
          printf("netmask: ");
          print_ip_info(xnetif.netmask);
          printf("gateway: ");
          print_ip_info(xnetif.gw);

          netif_dhcp_task_exist = 0;
          vTaskDelete(NULL);
		  return;
        } else {
          /* DHCP timeout */
          if (dhcp->tries > MAX_DHCP_TRIES) {
            DHCP_state = DHCP_TIMEOUT;

            /* Stop DHCP */
            dhcp_stop(&xnetif);

            /* Static address used */
            IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
            IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
            IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(&xnetif, &ipaddr , &netmask, &gw);

            printf("dhcp got ip fail\n");
            printf("ip address: ");
            print_ip_info(ipaddr);
            printf("netmask: ");
            print_ip_info(netmask);
            printf("gateway: ");
            print_ip_info(gw);

            netif_dhcp_task_exist = 0;
            vTaskDelete(NULL);
          }
        }
      }
      break;

      default: break;
    }
    /* wait 250 ms */
	vTaskDelay(250);
  }   
}

//
//	esp8266_output:
//		output datas to uart port
//		parameter:
//			start of data pointer
//			length of data
//		return:
//			total output data length or error code
//
int esp8266_output(unsigned char *buf, unsigned int len)
{
	return write(iSerialReceive, buf, len);
}

//
//	esp8266_input:
//		input datas from uart port
//		parameter:
//			none
//		return:
//			charater
//
int esp8266_input()
{
	int ch;
	printf("enter %s, queue = %p \n",__FUNCTION__, xSerialRxQueue);
	while (1) {
        // repeat read data and use timeout to exit
        if ( pdFALSE == xQueueReceive( xSerialRxQueue, (unsigned char *)&ch, 20 ) )  // wait more the 10ms, expired
        {
            if (esp8266_exit)
            {
				printf("%s: exit\n",__FUNCTION__);
				ch = -1;
                break;
            }
            continue;
        }
        break;
    }
    printf("exit %s %x\n",__FUNCTION__, ch);
    return ch;
}

//
//  function: esp8266_control
//      esp8266  module link control commands
//  parameters
//      1: enable esp8266 link, 0: disable esp8266 link
//
int esp8266_control(int action)
{
	if (esp8266_on==1 && action==0)
	{
		// terminate wifi session
		esp8266_exit = 1;
		printf("wait esp8266 terminate ...\n");
		if (xSemaphoreTake(esp8266_sem, 30000)!= pdTRUE)	
			printf("wait esp8266 terminate fail\n");
		netif_remove(&xnetif);
		lAsyncIOSerialClose(iSerialReceive);
        iSerialReceive = 0;
        vQueueDelete(xSerialRxQueue);
        printf("Close %s!\n",devname);
        esp8266_exit = 0;
        esp8266_on = 0;
        return 0;
	}
	else if (esp8266_on==0 && action == 1)
	{
		// initial esp8266 session
		if (hSerialTask!=NULL)
		{
			printf("esp8266 working\n");
			return -2;
		}
		if (esp8266_sem == NULL)
			vSemaphoreCreateBinary(esp8266_sem);
		
		if (esp8266_sem)
			xSemaphoreTake(esp8266_sem,0);
		// open uart device
		if ( pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
		{
			xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
			(void)lAsyncIORegisterCallback( iSerialReceive, vAsyncSerialIODataAvailableISR,
                                        xSerialRxQueue );
			printf("Open device %s success queue = %p\n",devname, xSerialRxQueue);
			esp8266_netinit(1);                  
			esp8266_on = 1;
		}
		else
			printf("Open %s fail\n",devname);
		return 0;
	}
	else
		return -1;
}

//
//  function: cmd_esp
//      esp 8266 control command
//  parameters
//      argc:   
//      argv:   
//
void cmd_esp(int argc, char* argv[])
{
	int c;
    int i;
    
     while((c=getopt(argc, argv, "d:b:m:ch?")) != -1)
    {
        switch(c)
        {
        case 'd':
            strncpy(devname, optarg, DEVICE_NAME_LEN);
            printf("set device %s\n",devname);
            break;
        case 'b':
            for(i=0; i<MAX_BAUD_NUM; i++)
            {
                if (strcmp(optarg,baudstr[i])==0)
                {
                    baudid = i;
                    printf("set baudrate %s\n",baudstr[baudid]);
                    break;
                }
            }
            break;
        case 'c':
            printf("clear counters\n");
            break;
        case 'h':
        case '?':
			printf("%s",curr_cmd->usage);
			break;
        default:
            printf("wrong command!\n usgae: %s\n",curr_cmd->usage);
            printf("%s",curr_cmd->usage);
            return;
        }
    }   // end while
    
    if (optind<argc)
    {
		for(;optind<argc;optind++)
		{
			if (strcmp(argv[optind],"on")==0)
				esp8266_control(1);
			else if (strcmp(argv[optind],"off")==0)
				esp8266_control(0);
			else
			{
				printf("%s",curr_cmd->usage);
				break;
			}
		}
	}
	else
		printf("esp8266 module %s, device %s, baudrate %s\n",
			(esp8266_on?"on":"off"),devname, baudstr[baudid]);

}

/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/



