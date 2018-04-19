
#include <unistd.h>
#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_RAW /* don't build if not configured for use in lwipopts.h */

#include "ping.h"

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/inet_chksum.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/debug.h"
#include "lwip/netdb.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmd.h"

/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif
/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif
/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif
/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif
/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

static xTaskHandle hPingTask;
static int repeat_time=4;
static u16_t ping_seq_num;
static u32_t ping_time;
//static struct raw_pcb *ping_pcb;

/** Prepare a echo ICMP request */
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
    size_t i;
    size_t data_len = len - sizeof(struct icmp_echo_hdr);

    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id     = PING_ID;
    iecho->seqno  = htons(++ping_seq_num);

    /* fill the additional data buffer with some data */
    for(i = 0; i < data_len; i++)
    {
        ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
    }

    iecho->chksum = inet_chksum(iecho, len);
}

/* Ping using the socket ip */
static err_t
ping_send(int s, ip_addr_t *addr)
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
    LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);
    LWIP_ASSERT("ping: expect IPv4 address", !IP_IS_V6(addr));

    iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
    if (!iecho)
    {
        return ERR_MEM;
    }

    ping_prepare_echo(iecho, (u16_t)ping_size);

    to.sin_len = sizeof(to);
    to.sin_family = AF_INET;
    inet_addr_from_ip4addr(&to.sin_addr, ip_2_ip4(addr));

    err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

    mem_free(iecho);

    return (err ? ERR_OK : ERR_VAL);
}

static void
ping_recv(int s)
{
    char buf[64];
    int fromlen, len;
    struct sockaddr_in from;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
    {
        if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr)))
        {
            if (from.sin_family != AF_INET)
            {
                LWIP_DEBUGF( PING_DEBUG, ("ping: invalid sin_family %d\n", from.sin_family));
            }
            else
            {
                ip4_addr_t fromaddr;
                inet_addr_to_ip4addr(&fromaddr, &from.sin_addr);
                LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
                ip4_addr_debug_print(PING_DEBUG, &fromaddr);
                LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\n", (sys_now() - ping_time)));

                iphdr = (struct ip_hdr *)buf;
                iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));
                if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num)))
                {
                    /* do some ping result processing */
                    PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
                    return;
                }
                else
                {
                    LWIP_DEBUGF( PING_DEBUG, ("ping: drop\n"));
                }
            }
        }
    }   // end of while

    if (len < 0)
    {
        LWIP_DEBUGF( PING_DEBUG, ("ping: recv - %"U32_F" ms - timeout\n", (sys_now()-ping_time)));
    }

    /* do some ping result processing */
    PING_RESULT(0);
}

/*-----------------------------------------------------------*/
void ping_thread( void *arg )
{
    int s,count=0;
    int timeout = PING_RCV_TIMEO;
    ip_addr_t ping_target;
    struct hostent *phe;
    char *addr = (char *)arg;

    // convert string to ip_addr_t
    if ((phe = gethostbyname(addr))!=NULL)
    {
		ip_addr_set_ip4_u32(&ping_target, *(u32_t *)phe->h_addr);
		ip_addr_debug_print_val(PING_DEBUG, ping_target);
	}
    else
    {
		printf("cannot get host %s address\n",addr);
		goto end_ping_thread;
	}
	//	ipaddr_aton(addr, &ping_target);
	
    if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0)
    {
        LWIP_DEBUGF( PING_DEBUG,("open socket error %"U32_F"\n",s));
        goto end_ping_thread;
    }

    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (count < repeat_time)
    {
        if (ping_send(s, &ping_target) == ERR_OK)
        {
			
            LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
            ip_addr_debug_print(PING_DEBUG, &ping_target);
            LWIP_DEBUGF( PING_DEBUG, (" %d\n", count));
			
            ping_time = sys_now();
            ping_recv(s);
        }
        else
        {
            LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
            ip_addr_debug_print(PING_DEBUG, &ping_target);
            LWIP_DEBUGF( PING_DEBUG, (" - error\n"));
        }
        vTaskDelay(PING_DELAY);
        count++;
    }
    lwip_close(s);
end_ping_thread:
    LWIP_DEBUGF( PING_DEBUG, ("ping: end "));
    // Kill init thread after all init tasks done
    hPingTask = NULL;
    vTaskDelete(NULL);
}

//
//  function: cmd_ping
//      send a ICMC ECHO request and wait server response
//  parameters
//      argc:   2
//      argv:   ping <ip address> -t <time>
//
void cmd_ping(int argc, char* argv[])
{
    int c;

    while((c=getopt(argc, argv, "t:")) != -1)
    {
        switch(c)
        {
        case 't':
            repeat_time = atoi(optarg);
            break;
        default:
            //printf("wrong command!\n usgae: %s\n",curr_cmd->usage);
            //return;
            break;
        }
    }   // end while
    if (optind < argc  && hPingTask==NULL)
    {
        xTaskCreate( ping_thread, "ping", configMINIMAL_STACK_SIZE, argv[optind], tskIDLE_PRIORITY + 1, &hPingTask );
    }
    else
        printf("ping is working\n");
}

#endif /* LWIP_IPV4 && LWIP_RAW */
