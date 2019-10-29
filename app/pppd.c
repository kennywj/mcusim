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
#include "sys.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSerial.h"

// network interface
#include "lwipopts.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "ethernetif.h"

#include "netif/ppp/ppp.h"
#include "netif/ppp/pppos.h"
#include "netif/ppp/pppapi.h"
#include "netif/ppp/ppp_impl.h"

static int ppp_on=0;
static xQueueHandle xSerialRxQueue;
static char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
static int iSerialReceive = 0;
static xTaskHandle hSerialTask;
static int baudid=3;	// default 115200

char PPP_User[256] = "test";
char PPP_Pass[256] = "test";

static int exit_ppp = 0, ppp_type =0;   // 0:client 1:server
unsigned int ppp_tx_pcnt, ppp_tx_bcnt, ppp_rx_pcnt, ppp_rx_bcnt;
static xSemaphoreHandle ppp_sem =	NULL;

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

//
//  function: link_control
//      ppp module link control commands
//  parameters
//      1: enable ppp link, 0: disable ppp link
//
int link_control(int action)
{
	if (ppp_on==1 && action==0)
	{
		// terminate ppp session
		exit_ppp = 1;
		printf("wait ppp terminate ...\n");
		if (xSemaphoreTake(ppp_sem, 30000)!= pdTRUE)	
			printf("wait ppp terminate fail\n");
		printf("Close %s!",devname);
		lAsyncIOSerialClose(iSerialReceive);
        iSerialReceive = 0;
        vQueueDelete(xSerialRxQueue);
        ppp_on = 0;
        return 0;
	}
	else if (ppp_on==0 && action == 1)
	{
		// initial ppp session
		if (hSerialTask!=NULL)
		{
			printf("PPP working\n");
			return -2;
		}
		if (ppp_sem == NULL)
			vSemaphoreCreateBinary(ppp_sem);
		
		if (ppp_sem)
			xSemaphoreTake(ppp_sem,0);
		// open uart device
		if ( pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
		{
			xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
			(void)lAsyncIORegisterCallback( iSerialReceive, vAsyncSerialIODataAvailableISR,
                                        xSerialRxQueue );
			printf("Open device %s success\n",devname);
			if (ppp_type)
				xTaskCreate( pppos_server_thread, "ppp_server", 4096, xSerialRxQueue,
                     tskIDLE_PRIORITY + 4, &hSerialTask );
			else 
				xTaskCreate( pppos_client_thread, "ppp_client", 4096, xSerialRxQueue,
                     tskIDLE_PRIORITY + 4, &hSerialTask );                    
			ppp_on = 1;
		}
		else
			printf("Open %s fail\n",devname);
		return 0;
	}
	else
		return -1;
}
//
//  function: cmd_ppp
//      ppp module control commands
//  parameters
//      argc:
//      argv:
//
void cmd_ppp(int argc, char* argv[])
{
    int c;
    int i;

    if (argc==1)
        goto ppp_help;
    
    while((c=getopt(argc, argv, "d:b:m:u:p:ch")) != -1)
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
            ppp_tx_pcnt= ppp_tx_bcnt= ppp_rx_pcnt= ppp_rx_bcnt =0;
            printf("clear counters\n");
            break;
        default:
            printf("wrong command!\n usgae: %s\n",curr_cmd->usage);
        case 'h':
ppp_help:        
        	printf("device %s, baudrate %s, PPP active %s\n",devname, baudstr[baudid], (ppp_on?"ON":"OFF"));
        	printf("PPP %s, username %s, password %s\n",(ppp_type?"server":"client"),PPP_User,PPP_Pass);
        	printf("PPP %s Tx %d(%dB), Rx %d(%dB)\n",(ppp_type?"server":"client"),
			ppp_tx_pcnt, ppp_tx_bcnt, ppp_rx_pcnt, ppp_rx_bcnt);
            return;
        }
    }   // end while
    
    for(;optind<argc;optind++)
	{
		if (strcmp(argv[optind],"on")==0)
			link_control(1);
		else if (strcmp(argv[optind],"off")==0)
			link_control(0);
	}
    
}
