#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sys.h"
#include "vphy.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSerial.h"

static xQueueHandle xSerialRxQueue;
static xSemaphoreHandle vphy_sem =	NULL;
static xTaskHandle hSerialTask=NULL;
static int iSerialReceive = 0;
static int vphy_on=0, exit_vphy =0;

unsigned int vphy_tx_pcnt, vphy_tx_bcnt, vphy_rx_pcnt, vphy_rx_bcnt;
char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
int baudid=3;	// default 115200

/**
  * @brief  virtual PHY default handler, dump received PDU
  * @param  rx queue handler
  * @retval None
  */
static void default_handler(char *pdu, int len)
{
	dump_frame(pdu, len,"[%4d]",vphy_rx_pcnt);
}

static void (*vphy_handler)(char *pdu, int len) = &default_handler;

/**
  * @brief  virtual PHY receive theread, handle the uart receive data
  * @param  rx queue handler
  * @retval None
  */
static void vphy_receive_thread( void *pvParameters )
{
    xQueueHandle hSerialRxQueue = ( xQueueHandle )pvParameters;
    char data[MAX_PKT_SIZE];
	unsigned char ch;
    int len;
    printf("Start VPHY Receive thread\n");
    while (1) {
        // repeat read data and use timeout to exit
        if ( pdFALSE == xQueueReceive( hSerialRxQueue, &ch, 20 ) )  // wait more the 10ms, expired
        {
            if (exit_vphy)
                break;
			// if more then 20ms no data, and data buffer not empty,
			// call handler to handle the receive data
			if (len)
			{	
				vphy_rx_pcnt += 1;
				if (vphy_handler)
					(vphy_handler)(data, len);
				len = 0;
			}
            continue;
        }
		vphy_rx_bcnt += 1;
        data[len++]=ch;
        if (len>=MAX_PKT_SIZE)
            len = MAX_PKT_SIZE-1;
    }  // end while
    printf( "Close %s! %s Task exiting.\n",devname,__FUNCTION__ );
    exit_vphy=0;
    hSerialTask = NULL;
    xSemaphoreGive(vphy_sem);
    vTaskDelete( NULL );    
}	// end of vphy_receive_thread


/**
  * @brief  virtual PHY output LL PDU API.
  * @param  data: PDU start pointer
  * @param  len: length of PDU
  * @retval size of write data (Bytes)
  */
int vphy_output(unsigned char *data, int len)
{
	//dump_frame(data,len,"PPP tx len %d\n",len);
	vphy_tx_pcnt += 1;
	vphy_tx_bcnt += len;
    return write(iSerialReceive, data, len);
}


/**
  * @brief  initial the virtual phy
  * @param  receive handler function
  * @retval size of write data (Bytes)
  */
int vphy_init(void (*handler)(char *, int len))
{
	// initial vphy session
	if (hSerialTask!=NULL)
	{
		printf("VPHY working\n");
		return -2;
	}
	if (vphy_sem == NULL)
		vSemaphoreCreateBinary(vphy_sem);
		
	if (vphy_sem)
		xSemaphoreTake(vphy_sem,0);
		// open uart device
	if ( pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
	{
		xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
		(void)lAsyncIORegisterCallback( iSerialReceive, vAsyncSerialIODataAvailableISR,
                                        xSerialRxQueue );
		printf("Open device %s success\n",devname);
		if (handler)
			vphy_handler = handler;
		xTaskCreate( vphy_receive_thread, "vphy", 4096, xSerialRxQueue,
            tskIDLE_PRIORITY + 4, &hSerialTask );
		vphy_on = 1;
	}
	else
		printf("Open %s fail\n",devname);
	return 0;
}

/**
  * @brief  terminate the virtual phy
  * @param  None
  * @retval 0: success, other fail
  */
int vphy_exit()
{
	// terminate VPHY
	exit_vphy = 1;
	printf("wait VPHY terminate ...\n");
	if (xSemaphoreTake(vphy_sem, 30000)!= pdTRUE)	
		printf("wait vphy terminate fail\n");
	printf("Close %s!",devname);
	lAsyncIOSerialClose(iSerialReceive);
    iSerialReceive = 0;
    vQueueDelete(xSerialRxQueue);
    vphy_on = 0;
    return 0;
}


/**
  * @brief  virtual phy control interface
  * @param  control option
  * @param  void pointer to data structure
  * @retval 0: success, other fail
  */
int vphy_ctrl(int opt, int len, void *buf)
{
	char *end = (char *)buf + len, *c=(char *)buf;
	
	switch(opt)
	{
		case 0:
			// get VPHY status
			c += snprintf(c, end-c,"\nVPHY: %s, status: %s\n", devname, (vphy_on?"active":"deactive"));
			c += snprintf(c, end-c,"TX: %08u Pkt, %08u bytes, RX: %08u Pkt, %08u bytes\n",
				vphy_tx_pcnt, vphy_tx_bcnt, vphy_rx_pcnt, vphy_rx_bcnt);
		break;
		case 1:
			// program real physical device
			if (len)
				strncpy(devname, buf, DEVICE_NAME_LEN);
		break;
		default:
			printf("Unknown option %d\n", opt);
		break;
	}
	return 0;
}
