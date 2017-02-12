#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
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
#include "cmd.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSerial.h"

struct _ethseg_msg_
{
    unsigned char start;
    unsigned char type;
    unsigned short len;
    unsigned char  buf[0];
};

#define DEVICE_NAME_LEN 64
#define MAX_PKT_SIZE    1520
#define MAX_MSG_SIZE    (MAX_PKT_SIZE-(sizeof(struct _ethseg_msg_)+2))
#define START_ID        0xf0
#define MAX_BAUD_NUM    4

static int u2w_on=0;
static char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
static int iSerialReceive = 0;
static xTaskHandle hSerialTask;
static xQueueHandle xSerialRxQueue;
static xSemaphoreHandle uart_tx_sem =	NULL;
static unsigned int eth_seg_tx_count=0, eth_seg_rx_count=0, eth_seg_rx_err=0;
static unsigned char eth_seg_state=0, txbuf[MAX_PKT_SIZE];
const char *baudstr[MAX_BAUD_NUM]={"9600","38400","57600","115200"};
const int baudrate[MAX_BAUD_NUM]={B9600,B38400,B57600,B115200};
int baudid=1;
extern unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);

//
//  function: uart_rx_process
//      main function to process the recv characters from uart device
//  parameters
//      argc:   1
//      argv:   none
//
void uart_rx_process( void *pvParameters )
{
    xQueueHandle hSerialRxQueue = ( xQueueHandle )pvParameters;
    unsigned char ch,cmd;
    unsigned char buf[MAX_PKT_SIZE], resp[MAX_MSG_SIZE];
    unsigned short off=0, len, crc, msgcrc;
    
	if ( NULL != hSerialRxQueue )
	{
		for ( ;; )
		{
			if ( pdFALSE == xQueueReceive( hSerialRxQueue, &ch, 20 ) )  // wait more the 20ms, expired
			{
			    eth_seg_state = 0;  // expired
		        off = 0;
		        continue;
			}
			//printf("%x(%c) ",ch,(ch>0x20?ch:'.'));
			buf[off++] = ch;
		    switch(eth_seg_state)
		    {
		        case 0:
		            if (ch==0xf0)
		               eth_seg_state = 1;
    		    break;
		        case 1:
		            if (ch == 0x00 || ch == 0x01 || ch == 0x81)
		            {    
		                eth_seg_state = 2;
		                cmd = ch;
		            }
		            else
		                eth_seg_state = 0;
		        break;        
		        case 2:
		            if (ch <= 0x05)
		            {
		                eth_seg_state = 3;
		                len = (ch<<8);
		            }    
		            else   
		                eth_seg_state = 0;
		        break; 
		        case 3:
		            len |= ch;
		            if (len <= 1514)    // max packet length
		                eth_seg_state = 4;
		            else   
		                eth_seg_state = 0;
		        break; 
		        case 4:
		            if (off==(len+sizeof(struct _ethseg_msg_)+2))   // include crc
		            {
		                eth_seg_state = 0;
		                // do crc check
		                msgcrc = *((unsigned short *)&buf[len+sizeof(struct _ethseg_msg_)]);
		                crc = CRC16(buf, len+sizeof(struct _ethseg_msg_));
		                //printf("%s: len=%d, crc=%x, orig crc=%x\n",__FUNCTION__, len, crc, msgcrc);
		                //dump_frame("",buf,len+sizeof(struct _ethseg_msg_)+2);
		                if (crc != msgcrc)
		                {
                    	    printf("crc error %x %x\n",crc, msgcrc);
                    	    eth_seg_rx_err++;
		                    break;        
		                }    
		                // forward packet
		                eth_seg_rx_count++;
		                if (cmd) // cmd or response
		                {
		                    buf[sizeof(struct _ethseg_msg_)+len]='\0';
		                    printf("%s:%s\n",(cmd==0x81?"response":"command"),(char *)&buf[sizeof(struct _ethseg_msg_)]);
		                    //if (cmd==0x01)
		                    //  do_command();
		                }    
		                else    
		                {   // data packet 
		                    dump_frame("receviced packet",(char *)&buf[sizeof(struct _ethseg_msg_)],len);
		                }
                    }                    	
		        break; 
		    }   // end of state switch  
		    if (eth_seg_state==0)
		        off = 0;
		}
	}

	/* Port wasn't opened. */
	printf( "%s Task exiting.\n",__FUNCTION__ );
	vTaskDelete( NULL );
}


//
//  function: uart_tx_process
//      main function of transmission ethernet packet or command,
//      it calling from upper layer, need synchronize protect
//      parameter: 
//          char type: 0: data, 1: command, 81: command response
//          int len: message length
//          char *msg: start of message
//
void uart_tx_process(char type, int len, char *data) 
{
    struct _ethseg_msg_ *msg = (struct _ethseg_msg_ *)txbuf;
    unsigned short crc;
    int ret,size;
    
    if (len > (MAX_PKT_SIZE-(sizeof(struct _ethseg_msg_)+2)))
    {
	    printf("size=%d overflow\n",len);
        return;
    } 
    if (iSerialReceive<=0 || uart_tx_sem==NULL)
    {
        printf("device not ready");
        return;
    }   
    // obtain mutex
    xSemaphoreTake(uart_tx_sem, portMAX_DELAY);
    // contruct output message
    msg->start = START_ID;
    msg->type = type;
    msg->len = htons(len);
    memcpy(msg->buf, data, len);
    // do crc calculate, from start tag to endo of payload
    crc = CRC16((unsigned char *)msg, len + sizeof(struct _ethseg_msg_));
    memcpy((char *)&msg->buf[len],(char *)&crc, sizeof(unsigned short));
    size = len + sizeof(struct _ethseg_msg_) + 2;
    // transmit
    //printf("%s: len=%d, crc=%x, sizeof %d\n",__FUNCTION__, len, crc, sizeof(struct _ethseg_msg_));
    //dump_frame("",msg,len + sizeof(struct _ethseg_msg_) + 2);
    ret = write(iSerialReceive, msg, size); // include crc
    if (ret != size)
    {
        if (ret == -1)
            printf("write error %d\n",errno);
        else
            printf("ret=%d, lost %d\n",ret, size-ret);
    }    
    eth_seg_tx_count++;
    xSemaphoreGive(uart_tx_sem);
}

//
//  function: cmd_stat
//      diaplay status of tunnel server daemon
//  parameters
//      argc:   1
//      argv:   none
//

void cmd_stat(int argc, char* argv[])
{
    printf("device %s, baudrate %s, %s\n",devname, baudstr[baudid], (u2w_on?"ON":"OFF"));
    printf("Tx %d, Rx %d, Error %d\n",eth_seg_tx_count, eth_seg_rx_count, eth_seg_rx_err);
}

//
//  function: cmd_on
//      enable wifi to uart device
//  parameters
//      argc:   1
//      argv:   none
//

void cmd_on(int argc, char* argv[])
{
    if (u2w_on)
    {
        printf("Actived!");    
        return;
    }
    
    uart_tx_sem =  xSemaphoreCreateMutex();
	if (uart_tx_sem == NULL)
	{
		printf("strat uart Tx semaphore fail\n");
		return;
	}
	
	// open uart device
    if ( pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
    {
        xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
        (void)lAsyncIORegisterCallback( iSerialReceive, vAsyncSerialIODataAvailableISR, 
            xSerialRxQueue );
        printf("Open device %s success\n",devname);
        /* Create a Task which waits to receive bytes. */
	    xTaskCreate( uart_rx_process, "uartrx", 4096, xSerialRxQueue, 
	        tskIDLE_PRIORITY + 4, &hSerialTask );
	    u2w_on = 1;    
    }    
}
    
//
//  function: cmd_off
//      disable wifi to uart device
//  parameters
//      argc:   1
//      argv:   none
//

void cmd_off(int argc, char* argv[])
{
	u2w_on = 0;    
    printf("has not implement %s\n",__FUNCTION__);
}
    
//
//  function: cmd_cfg
//      diaplay and program local device ip address
//  parameters
//      argc:   2
//      argv:   -r<report time, unit seconds, 0 disable)
//
void cmd_cfg(int argc, char* argv[])
{
    int c;
    int i;
    
    if (argc==1)
    {
        printf("device %s, baudrate %s, %s\n",devname, baudstr[baudid], (u2w_on?"ON":"OFF"));
        return;
    }    
    while((c=getopt(argc, argv, "p:b:")) != -1)
    {
        switch(c)
        {
            case 'p':
                strncpy(devname, optarg, DEVICE_NAME_LEN);
                printf("set device %s\n",devname);
            break;
            case 'b':
                for(i=0;i<MAX_BAUD_NUM;i++)
                {
                    if (strcmp(optarg,baudstr[i])==0)
                    {    
                       baudid = i;
                       printf("set baudrate %s\n",baudstr[baudid]);
                       break;
                    }   
                }
            break;
            default:
                printf("wrong command!\n usgae: %s\n",curr_cmd->usage);
            return;
        }
    }   // end while
}

//
//  function: cmd_xmt
//      xmt a message to peer via UART
//  parameters
//      argc:   1
//      argv:   none
//

void cmd_xmt(int argc, char* argv[])
{
    if (argc>1)
        uart_tx_process(1, strlen(argv[1]), argv[1]);
    else
        printf("No argument!\nUsage: %s\n",curr_cmd->usage);    
}