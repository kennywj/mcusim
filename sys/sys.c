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

// network interface
#include "lwipopts.h"
#include "lwip/err.h"
#include "ethernetif.h"

#if PPP_SUPPORT
#include "netif/ppp/ppp.h"
#include "netif/ppp/pppos.h"
#include "netif/ppp/pppapi.h"
#include "netif/ppp/ppp_impl.h"
#endif

#define MAX_PKT_SIZE    1520
#define MAX_BAUD_NUM    4
#define DEVICE_NAME_LEN 64

static int u2w_on=0;
static xQueueHandle xSerialRxQueue;
static char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
static int iSerialReceive = 0;
static xTaskHandle hSerialTask;
const char *baudstr[MAX_BAUD_NUM]= {"9600","38400","57600","115200"};
const int baudrate[MAX_BAUD_NUM]= {B9600,B38400,B57600,B115200};
int baudid=3;	// default 115200
unsigned char debug_flags = (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT);


extern int xmodemTransmit(unsigned char *src, int srcsz);
extern int xmodemReceive(unsigned char *dest, int destsz);

#if PPP_SUPPORT
const char PPP_User[256] = "test";
const char PPP_Pass[256] = "test";

static int exit_ppp = 0, ppp_type =0;   // 0:client 1:server
unsigned int ppp_tx_pcnt, ppp_tx_bcnt, ppp_rx_pcnt, ppp_rx_bcnt;
static xSemaphoreHandle ppp_sem =	NULL;
#else
// not PPP_SUPPORT
struct _ethseg_msg_
{
    unsigned char start;
    unsigned char type;
    unsigned short len;
    unsigned char  buf[0];
};

#define MAX_MSG_SIZE    (MAX_PKT_SIZE-(sizeof(struct _ethseg_msg_)+2))
#define START_ID        0xf0

static xSemaphoreHandle uart_tx_sem =	NULL;
static unsigned int eth_seg_tx_count=0, eth_seg_rx_count=0, eth_seg_rx_err=0, eth_seg_rx_drop=0;
static unsigned char eth_seg_state=0, txbuf[MAX_PKT_SIZE];

extern unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);
#endif
// end of PPP_SUPPORT

//
// input character from uart
//
int _inbyte(int msec)
{
    int ch;
    // wait 10 ticks = 10ms?
    while ( pdFALSE == xQueueReceive( xSerialRxQueue, &ch, 10 ) )
    {
        msec-=10;
        if (msec <= 0)
			return -1;
    }
    return ch;
}

//
// output character to uart
//
void _outbyte(unsigned char c)
{
    write(iSerialReceive, &c, 1); // include crc
}



#if PPP_SUPPORT

/* PPP status callback example */
static void ppp_status_cb(ppp_pcb *pcb, int err_code, void *ctx)
{
    struct netif *pppif = ppp_netif(pcb);
    LWIP_UNUSED_ARG(ctx);

    switch (err_code) 
    {
        case PPPERR_NONE: {
#if LWIP_DNS
            const ip_addr_t *ns;
#endif /* LWIP_DNS */            
            printf("status_cb: Connected\n");
#if PPP_IPV4_SUPPORT
            printf("   our_ipaddr  = %s\n", ipaddr_ntoa(&pppif->ip_addr));
            printf("   his_ipaddr  = %s\n", ipaddr_ntoa(&pppif->gw));
            printf("   netmask     = %s\n", ipaddr_ntoa(&pppif->netmask));
    #if LWIP_DNS
            ns = dns_getserver(0);
            printf("   dns1        = %s\n", ipaddr_ntoa(ns));
            ns = dns_getserver(1);
            printf("   dns2        = %s\n", ipaddr_ntoa(ns));
    #endif /* LWIP_DNS */            
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
            printf("   our6_ipaddr = %s\n", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
#endif /* PPP_IPV6_SUPPORT */

            break;
        }
        case PPPERR_PARAM: {
            printf( "status_cb: Invalid parameter\n");
            break;
        }
        case PPPERR_OPEN: {
            printf( "status_cb: Unable to open PPP session\n");
            break;
        }
        case PPPERR_DEVICE: {
            printf( "status_cb: Invalid I/O device for PPP\n");
            break;
        }
        case PPPERR_ALLOC: {
            printf( "status_cb: Unable to allocate resources\n");
            break;
        }
        case PPPERR_USER: {
            printf( "status_cb: User interrupt\n");
            break;
        }
        case PPPERR_CONNECT: {
            printf( "status_cb: Connection lost\n");
            break;
        }
        case PPPERR_AUTHFAIL: {
            printf( "status_cb: Failed authentication challenge\n");
            break;
        }
        case PPPERR_PROTOCOL: {
            printf( "status_cb: Failed to meet protocol\n");
            break;
        }
        case PPPERR_PEERDEAD: {
            printf( "status_cb: Connection timeout\n");
            break;
        }
        case PPPERR_IDLETIMEOUT: {
            printf( "status_cb: Idle Timeout\n");
            break;
        }
        case PPPERR_CONNECTTIME: {
            printf( "status_cb: Max connect time reached\n");
            break;
        }
        case PPPERR_LOOPBACK: {
            printf( "status_cb: Loopback detected\n");
            break;
        }
        default: {
            printf( "status_cb: Unknown error code %d\n", err_code);
            break;
        }
    }   // end switch

    /*
     * This should be in the switch case, this is put outside of the switch
     * case for example readability.
     */

    if (err_code == PPPERR_NONE) {
        return;
    }

    /* ppp_close() was previously called, don't reconnect */
    if (err_code == PPPERR_USER) {
        /* ppp_free(); -- can be called here */
        ppp_free(pcb);
        exit_ppp = 2;
        return;
    }


    /*
     * Try to reconnect in 30 seconds, if you need a modem chatscript you have
     * to do a much better signaling here ;-)
     */
    if (ppp_type==0)
        ppp_connect(pcb, 30);
    /* OR ppp_listen(pcb); */
}

static u32_t ppp_output_callback(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
	//dump_frame(data,len,"PPP tx len %d\n",len);
	ppp_tx_pcnt += 1;
	ppp_tx_bcnt += len;
    return write(iSerialReceive, data, len);
    //return uart_write_bytes(uart_num, (const char *)data, len);
}

//
//  function: pppos_client_thread
//      main function to process the recvived PPP packet from uart device 
//  parameters
//      argc:   1
//      argv:   none
//
void pppos_client_thread( void *pvParameters )
{
	/* The PPP control block */
	ppp_pcb *ppp;
	/* The PPP IP interface */
	struct netif ppp_netif;
    xQueueHandle hSerialRxQueue = ( xQueueHandle )pvParameters;
    char data[MAX_PKT_SIZE];
    u8_t nocarrier = 0, ch;
    int len;
    
    printf("PPP client start\n");
    
    // init PPP over serial
    ppp = pppos_create(&ppp_netif,
        ppp_output_callback, ppp_status_cb, NULL);

    if (ppp == NULL) {
        printf("Error init pppos\n");
        goto end_ppp_client;
    }

    ppp_set_default(ppp);

    /* Ask the peer for up to 2 DNS server addresses. */
    ppp_set_usepeerdns(ppp, 1);

    ppp_set_auth(ppp, PPPAUTHTYPE_PAP, PPP_User, PPP_Pass);

    ppp_connect(ppp, 0);

    printf("After ppp_connect\n");
    
    exit_ppp = 0;
    len = 0;

    while (1) {
        // repeat read data and use timeout to exit
        if ( pdFALSE == xQueueReceive( hSerialRxQueue, &ch, 20 ) )  // wait more the 10ms, expired
        {
            if (exit_ppp)
                break;
            continue;
        }
        
		ppp_rx_bcnt += 1;
        if (ch==PPP_FLAG)
        {
			if (len>1)
			{
				data[len++]=ch;	// end of frame
				//dump_frame(data,len,"PPP rx len %d\n",len);
        		ppp_rx_pcnt += 1;
                pppos_input_tcpip(ppp, (u8_t *)data, len);
			}
			len = 0;
		}    
        data[len++]=ch;
        if (len>=MAX_PKT_SIZE)
            len = MAX_PKT_SIZE-1;
     }  // end while
end_ppp_client:
    if (ppp)
        ppp_close(ppp, nocarrier);  
         
    while(exit_ppp != 2)
    {
		vTaskDelay(1000);	// delay 1 second
    }
    printf( "Close %s! %s Task exiting.\n",devname,__FUNCTION__ );
    exit_ppp=0;
    hSerialTask = NULL;
    xSemaphoreGive(ppp_sem);
    vTaskDelete( NULL );
}



//
//  function: pppos_server_thread
//      act as a PPP server
//      main function to process the recvived PPP packet from uart device 
//  parameters
//      argc:   1
//      argv:   none
//
void pppos_server_thread( void *pvParameters )
{
	/* The PPP control block */
	ppp_pcb *ppp;
	/* The PPP IP interface */
	struct netif ppp_netif;
    xQueueHandle hSerialRxQueue = ( xQueueHandle )pvParameters;
    char data[MAX_PKT_SIZE];
    u8_t nocarrier = 0, ch;
    int len;
    
    // do Lwip init (include PPP)
    //LwIP_Init();	// move to main.c
    
    // init PPP over serial
    ppp = pppos_create(&ppp_netif,
        ppp_output_callback, ppp_status_cb, NULL);

    printf("After pppapi_pppos_create\n");

    if (ppp == NULL) {
        printf("Error init pppos\n");
        goto end_ppp_server;
    }
    
    /*
    * Basic PPP server configuration. Can only be set if PPP session is in the
    * dead state (i.e. disconnected). We don't need to provide thread-safe
    * equivalents through PPPAPI because those helpers are only changing
    * structure members while session is inactive for lwIP core. Configuration
    * only need to be done once.
    */
    ip4_addr_t addr;

    printf("PPP server\n");
    /* Set our address */
    IP4_ADDR(&addr, 192,168,0,1);
    ppp_set_ipcp_ouraddr(ppp, &addr);

    /* Set peer(his) address */
    IP4_ADDR(&addr, 192,168,0,2);
    ppp_set_ipcp_hisaddr(ppp, &addr);
#if LWIP_DNS
    /* Set primary DNS server */
    IP4_ADDR(&addr, 192,168,10,20);
    ppp_set_ipcp_dnsaddr(ppp, 0, &addr);

    /* Set secondary DNS server */
    IP4_ADDR(&addr, 192,168,10,21);
    ppp_set_ipcp_dnsaddr(ppp, 1, &addr);
#endif
    /* Auth configuration, this is pretty self-explanatory */
    ppp_set_auth(ppp, PPPAUTHTYPE_ANY, PPP_User, PPP_Pass);

    /* Require peer to authenticate */
    ppp_set_auth_required(ppp, 1);

    /*
    * Only for PPPoS, the PPP session should be up and waiting for input.
    *
    * Note: for PPPoS, ppp_connect() and ppp_listen() are actually the same thing.
    * The listen call is meant for future support of PPPoE and PPPoL2TP server
    * mode, where we will need to negotiate the incoming PPPoE session or L2TP
    * session before initiating PPP itself. We need this call because there is
    * two passive modes for PPPoS, ppp_set_passive and ppp_set_silent.
    */
    ppp_set_silent(ppp, 1);

    /*
    * Initiate PPP listener (i.e. wait for an incoming connection), can only
    * be called if PPP session is in the dead state (i.e. disconnected).
    */
    ppp_listen(ppp);
    printf("PPP server ready, waiting connect from client\n");
    exit_ppp = 0;
    len = 0;

    while (1) {
        // repeat read data and use timeout to exit
        if ( pdFALSE == xQueueReceive( hSerialRxQueue, &ch, 20 ) )  // wait more the 10ms, expired
        {
            if (exit_ppp)
                break;
            continue;
        }
        
		ppp_rx_bcnt += 1;
        if (ch==PPP_FLAG)
        {
			if (len>1)
			{
				data[len++]=ch;	// end of frame
				//dump_frame(data,len,"PPP rx len %d\n",len);
				ppp_rx_pcnt += 1;
                pppos_input_tcpip(ppp, (u8_t *)data, len);
			}
			len = 0;
		}  
        data[len++]=ch;
        if (len>=MAX_PKT_SIZE)
            len = MAX_PKT_SIZE-1;
     }  // end while
end_ppp_server:
    if (ppp)
    {
        ppp_close(ppp, nocarrier);   
        ppp_free(ppp);
    }
    printf( "Close %s! %s Task exiting.\n",devname,__FUNCTION__ );
    exit_ppp=0;
    hSerialTask = NULL;
    xSemaphoreGive(ppp_sem);
    vTaskDelete( NULL );    
}


#else
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
    struct netif *uartif=NULL;
    int ret;
    

    if ( NULL != hSerialRxQueue )
    {
        // do Lwip init
        //uartif = LwIP_Init();
        uartif = init_netifs();
        if (!uartif)
            goto end_uart_rx_process;

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
                        //  do_command();
                    }
                    else
                    {
                        // data packet
                        //dump_frame("receviced packet",(char *)&buf[sizeof(struct _ethseg_msg_)],len);
                        // forward to lwip
                        ret = ethernetif_recv(uartif, (char *)&buf[sizeof(struct _ethseg_msg_)],len);
                        if (ret<0)
                            eth_seg_rx_drop++;

                    }
                }
                break;
            }   // end of state switch
            if (eth_seg_state==0)
                off = 0;
        }
    }
end_uart_rx_process:
    /* Port wasn't opened. */
    printf( "%s Task exiting.\n",__FUNCTION__ );
    hSerialTask = NULL;
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
        printf("%s: size=%d overflow\n",__FUNCTION__,len);
        return;
    }
    if (iSerialReceive<=0 || uart_tx_sem==NULL)
    {
        printf("%s: device not ready\n",__FUNCTION__);
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
    dump_frame(msg,len + sizeof(struct _ethseg_msg_) + 2,"%s:%d uart tx len=%d\n",__FUNCTION__,__LINE__, len);
    ret = write(iSerialReceive, msg, size); // include crc
    if (ret != size)
    {
        if (ret == -1)
            printf("%s: write error %d\n",__FUNCTION__,errno);
        else
            printf("%s: ret=%d, lost %d\n",__FUNCTION__,ret, size-ret);
    }
    eth_seg_tx_count++;
    xSemaphoreGive(uart_tx_sem);
}
#endif
// end of PPP_SUPPORT

//
//  function: exit_u2w
//      do exit of uart2wifi module
//  parameters:
//      int pointer on_off
//  return:
//      none
//
void exit_u2w()
{
    if (u2w_on)
    {
		u2w_on = 0;
#if PPP_SUPPORT        
		exit_ppp = 1;
		printf("wait ppp terminate ...\n");
		if (xSemaphoreTake(ppp_sem, 30000)!= pdTRUE)	
		{
			printf("wait ppp terminate fail\n");
		}
#endif
		printf("Close %s!",devname);
		lAsyncIOSerialClose(iSerialReceive);
        iSerialReceive = 0;
        vQueueDelete(xSerialRxQueue);
    }
}

//
//  function: cmd_info
//      diaplay status of tunnel server daemon
//  parameters
//      argc:   1
//      argv:   none
//

void cmd_info(int argc, char* argv[])
{
    printf("device %s, baudrate %s, %s\n",devname, baudstr[baudid], (u2w_on?"ON":"OFF"));
#if PPP_SUPPORT    
    printf("PPP %s Tx %d(%dB), Rx %d(%dB)\n",(ppp_type?"server":"client"),
        ppp_tx_pcnt, ppp_tx_bcnt, ppp_rx_pcnt, ppp_rx_bcnt);
#else
// not PPP_SUPPORT
    printf("Tx %d, Rx %d, Error %d, Drop %d\n",eth_seg_tx_count, eth_seg_rx_count, eth_seg_rx_err, eth_seg_rx_drop);
#endif
// PPP_SUPPORT
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
        printf("Actived!\n");
        return;
    }
#if !PPP_SUPPORT
    uart_tx_sem =  xSemaphoreCreateMutex();
    if (uart_tx_sem == NULL)
    {
        printf("strat uart Tx semaphore fail\n");
        return;
    }
#else
	if (hSerialTask!=NULL)
	{
		printf("PPP working\n");
		return;
	}
	if (ppp_sem == NULL)
		vSemaphoreCreateBinary(ppp_sem);
		
	if (ppp_sem)
		xSemaphoreTake(ppp_sem,0);
#endif
    // open uart device
    if ( pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
    {
        xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
        (void)lAsyncIORegisterCallback( iSerialReceive, vAsyncSerialIODataAvailableISR,
                                        xSerialRxQueue );
        printf("Open device %s success\n",devname);
#if PPP_SUPPORT 
        if (ppp_type)
            xTaskCreate( pppos_server_thread, "ppp_server", 4096, xSerialRxQueue,
                     tskIDLE_PRIORITY + 4, &hSerialTask );
        else 
            xTaskCreate( pppos_client_thread, "ppp_client", 8192, xSerialRxQueue,
                     tskIDLE_PRIORITY + 4, &hSerialTask );
#else
        /* Create a Task which waits to receive bytes. */
        xTaskCreate( uart_rx_process, "uartrx", 4096, xSerialRxQueue,
                     tskIDLE_PRIORITY + 4, &hSerialTask );
#endif                     
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
	exit_u2w();
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
        printf("PPP %s, username %s, password %s\n",(ppp_type?"server":"client"),PPP_User,PPP_Pass);
        return;
    }
    while((c=getopt(argc, argv, "d:b:m:u:p:c")) != -1)
    {
        switch(c)
        {
		case 'u':
			strncpy(PPP_User,optarg,255);
			printf("set username %s\n",PPP_User);
			break;
	    case 'p':
			strncpy(PPP_Pass,optarg,255);
			printf("set password %s\n",PPP_Pass);
			break;
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
        case 'm':
            ppp_type = atoi(optarg)&0x01;
            printf("set work mode PPP %s\n",ppp_type?"server":"client");
            break;
        case 'c':
#if PPP_SUPPORT         
            ppp_tx_pcnt= ppp_tx_bcnt= ppp_rx_pcnt= ppp_rx_bcnt =0;
            printf("clear counters\n");
            break;
#endif            
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
#if !PPP_SUPPORT 
void cmd_xmt(int argc, char* argv[])
{
    if (argc>1)
        uart_tx_process(1, strlen(argv[1]), argv[1]);
    else
        printf("No argument!\nUsage: %s\n",curr_cmd->usage);
}
#endif

//
// enable xmodemint u2w_on
//
void cmd_xmodem(int argc, char* argv[])
{
    int c, ret=0, op=-1;
    char *tbuf=NULL;
    static unsigned int addr=0, len=0x10000;
    
    while ((c = getopt (argc, argv, "a:l:rw?")) != -1)
    {
	    switch (c) {
    	case 'r':
    	    op = 0;
	    break;
	    case 'w':
	        op = 1;
	    break;
	    case 'a':
	        addr = atoi(optarg);
	    break;
	    case 'l':
	        len = atoi(optarg);
	    break;
	    case '?':
	    default:
	        printf("usage: -r(read data from console) | -w (write data to console)>\n");
	        goto end_xmodem;
	    }
    }   // end of while
    
    if (op == -1)
    {
        printf("Xmodem setting: mode=%s, addr=%p, size = %d\n",(op?"read":"write"),addr, len);
        goto end_xmodem;
    }
    
    if (u2w_on)
    {
        printf("serial port inuse\n");
        goto end_xmodem;
    }
    
    if (op==0)
    {
        tbuf = malloc(len+128); // avoid boudary block need extra one
        if (!tbuf)
        {
            printf("heap not enough, require=%d\n",len);    
            goto end_xmodem;
        } 
    } 
    // open uart device
    if ( pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
    {
        xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
        (void)lAsyncIORegisterCallback( iSerialReceive, vAsyncSerialIODataAvailableISR,
                                        xSerialRxQueue );
        printf("Open device %s success\n",devname);
        
        u2w_on = 1;
        
        if (op==0)
        {    
            ret = xmodemReceive((unsigned char *)tbuf, len);
            if (ret < 0) {
		        printf("Xmodem receive error: status: %d\n", ret);
	        }
	        else  
	        {
   		        dump_frame(tbuf,ret,"receive data\n");
		        printf("Xmodem successfully received data len=%d bytes\n", ret);
	        }
        }
        else if (op==1)
        {
            if (u2w_on)
            {
                printf("serial port inuse\n");
                goto end_xmodem;
            }
        
            if (len)
            {   
                ret = xmodemTransmit((unsigned char *)addr, len);
    	        if (ret < 0)
	    	        printf("Xmodem transmit error: status: %d\n", ret);
	            else  
		            printf("Xmodem successfully transmitted from %p len=%d bytes\n", addr, ret);
		    }
		    else
		        printf("Xmodem transmit error: no length\n");
        }  
        // close device
        exit_u2w(); 
    }   
end_xmodem:	
     if (tbuf)
        free(tbuf);
    return;
}
