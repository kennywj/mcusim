/* states */
#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ff.h"
#include "fs_port.h"
#include "cmd.h"

extern int netif_info(char *buf, int size);
//
//  function: cmd_net
//      display network interface information
//  parameters
//      argc:   1
//      argv:   none
//
void cmd_net(int argc, char* argv[])
{
	char buf[512];
	if (netif_info(buf, 512)==0)
		printf("%s",buf);
}

//
//  function: client_thread
//      https clinet thread, call http client function in mbedtls
//  parameters
//      argc:   1
//      argv:   param
//
static xTaskHandle hHttpcTask;
static char server_url[256]="localhost";
static unsigned short server_port=4433;
static int httpc_on = 0, tls_on = 1;
static xSemaphoreHandle httpc_sem =	NULL;

static void client_thread( void *pvParameters )
{
	extern void https_client(const char *server_url, unsigned short port);
	
	https_client(server_url, server_port);
	
	xSemaphoreGive(httpc_sem);
	vTaskDelete( NULL ); 
}


//
//  function: http_clinet_ctrl
//      start a thread to do https clinet
//  parameters
//      argc:   1
//      argv:   none
//
void http_client_ctrl(int ctrl)
{
	if (httpc_on==0 && ctrl == 1)
	{
		if (httpc_sem == NULL)
			vSemaphoreCreateBinary(httpc_sem);
		if (httpc_sem)
			xSemaphoreTake(httpc_sem,0);
		if (xTaskCreate( client_thread, "http_clinet", 4096, NULL,
			tskIDLE_PRIORITY + 4, &hHttpcTask )!= pdTRUE)
        {
			printf("cannot create http clinet thread\n");
			return;
		}                     
        httpc_on=1;
	}
	else if (httpc_on==1 && ctrl == 0)
	{
		if (xSemaphoreTake(httpc_sem, 30000)!= pdTRUE)	
			printf("wait httpc_sem terminate fail\n");
		httpc_on=0;
	}
}

//
//  function: cmd_http_client
//      comamnd for http client
//  parameters
//      argc:   1
//      argv:   none
//
void cmd_http_client(int argc, char* argv[])
{
	int c;
	// start a thread to do ssl client 
	while((c=getopt(argc, argv, "u:p:h?")) != -1)
    {
        switch(c)
        {
		case 'u':
			strncpy(server_url,optarg,255);
			printf("set server %s\n",server_url);
			break;
	    case 'p':
			server_port = atoi(optarg);
			printf("set port %u\n",server_port);
			break;
        default:
            printf("wrong command!\n");
		case 'h':
		case '?':
			printf("usage: %s\n",curr_cmd->usage);
			return;
        }
    }   // end while
    if (optind==argc)
    {
		printf("http server %s, port %u, TLS %d\n",server_url, server_port, tls_on);
		return;
	}
	
    for(;optind<argc;optind++)
	{
		if (strcmp(argv[optind],"on")==0)
			http_client_ctrl(1);
		else if (strcmp(argv[optind],"off")==0)
			http_client_ctrl(0);
	}
}
