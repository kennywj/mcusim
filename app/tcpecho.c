#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <lwip/opt.h>
#if LWIP_IPV4

#include <lwip/sockets.h> 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmd.h"

#define ECHO_PORT 7777
#define ECHO_RCV_TIMEO 3000
static int echo_client_on =0;
static int echo_time = 1;
static unsigned short echo_port = ECHO_PORT;
static char echo_server[256]="127.0.0.1";

static xTaskHandle hTcpEchoServerTask,hTcpEchoClientTask;
//
// tcp echo server 
//
void tcpecho_server_handler(void *arg)
{
	//unsigned short port = *(unsigned short *)arg;
	int socket_fd=-1,accept_fd=-1;
	int sent_data, recv_data; 
	unsigned int addr_size;
	char data_buffer[80], send_buff[256]; 
	struct sockaddr_in sa,ra,isa;

	/* Creates an TCP socket (SOCK_STREAM) with Internet Protocol Family (PF_INET).
	* Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
	*/

	socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if ( socket_fd < 0 )
	{
		printf("socket call failed");
		goto end_tcpecho_server;
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("0.0.0.0");
	sa.sin_port = htons(ECHO_PORT);


	/* Bind the TCP socket to the port SENDER_PORT_NUM and to the current
	* machines IP address (Its defined by SENDER_IP_ADDR). 
	* Once bind is successful for UDP sockets application can operate
	* on the socket descriptor for sending or receiving data.
	*/
	if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
	{
		printf("Bind to Port Number %d failed\n",ECHO_PORT);
		goto end_tcpecho_server;
	}

	listen(socket_fd,1);
	while(1)
	{
		addr_size = sizeof(isa);
		accept_fd = accept(socket_fd, (struct sockaddr*)&isa,&addr_size);
		if(accept_fd < 0)
		{
			printf("accept failed\n");
			goto end_tcpecho_server;
		}
	
		while(1)
		{
			recv_data = recv(accept_fd,data_buffer,sizeof(data_buffer),0);
			if(recv_data <= 0)
			{
				if (recv_data==0)
				{
					printf("client close\n");
					break;
				}
				printf("receive data error %d\n",recv_data);
				break;
			}
			printf("%s:Server recv %d: \"%s\"\n",__FUNCTION__, recv_data, data_buffer);
			snprintf(send_buff,255,"Server get %d \"%s\"\n",recv_data,data_buffer);
			sent_data = send(accept_fd, send_buff,strlen(send_buff),0);
			if(sent_data < 0 )
			{
				printf("Server send failed %d\n",sent_data);
				break;
			}
			printf("%s:Server sent %d:\"%s\"\n",__FUNCTION__, sent_data, send_buff);
		} // end while
		close(accept_fd);
	}	// end while
end_tcpecho_server:
	if(socket_fd>=0)
		close(socket_fd);
	// Kill init thread after all init tasks done
    hTcpEchoServerTask = NULL;
    vTaskDelete(NULL);
}

//
// tcp echo client 
//
void tcpecho_client_handler(void *arg)
{
	char *msg = (char *)arg;
	int count, timeout = ECHO_RCV_TIMEO;
	int socket_fd=-1, ret;
	int addr_size,sent_data, recv_data; 
	char data_buffer[256]={0}, send_buff[256]={0}; 
	struct sockaddr_in sa,ra;
	
	/* Creates an TCP socket (SOCK_STREAM) with Internet Protocol Family (PF_INET).
	* Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
	*/
	printf("TCP echo client start\n");
	socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( socket_fd < 0 )
	{
		printf("socket call failed");
		goto end_tcpecho_client;
	}
	memset(&ra, 0, sizeof(struct sockaddr_in));
	ra.sin_family = AF_INET;
	ra.sin_addr.s_addr = inet_addr(echo_server);
	ra.sin_port = htons(ECHO_PORT);
	// set recv timeout
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	
	/* Connect to remote TCP server */
	ret = connect(socket_fd, (struct sockaddr *)&ra, sizeof(sa));
	if (ret == -1) {
		/* Close socket */
		printf("connect to server fail\n");
		goto end_tcpecho_client;
	}
	count =0;
	while(echo_time)
	{
		snprintf(send_buff,255,"%s: %d",msg, count++);
		printf("%s:Client sent %d:\"%s\"\n",__FUNCTION__, strlen(send_buff), send_buff);
		sent_data = send(socket_fd, send_buff,strlen(send_buff),0);
		if (sent_data<0)
		{
			printf("send data fail %d\n",sent_data);
			break;
		}
		recv_data = recv(socket_fd,data_buffer,sizeof(data_buffer),0);
		if(recv_data < 0)
		{
			printf("receive data error %d\n",recv_data);
			break;
		}
		printf("%s:Client recv %d:\"%s\"\n",__FUNCTION__,recv_data,data_buffer);
		echo_time--;
		vTaskDelay(1000);	// delay 1 second
	}
end_tcpecho_client:
	if(socket_fd>=0)
		close(socket_fd);
	// Kill init thread after all init tasks done
	printf("TCP echo client end\n");
    hTcpEchoClientTask = NULL;
    echo_client_on =0;
    vTaskDelete(NULL);
}

//  function: cmd_echo
//      send message to echo server and get reply from server
//  parameters
//      argc:   2
//      argv:   echo <message>
//
void cmd_echo(int argc, char* argv[])
{
    int c, go=0;
    static char message[256]="test";
    
    while((c=getopt(argc, argv, "t:p:s:m:gh?")) != -1)
    {
        switch(c)
        {
			case 't':
				echo_time = atoi(optarg);
			break;
			case 'p':
				echo_port = atoi(optarg);
			break;
			case 'g':
				go =1;
			break;
			case 's':
				strncpy(echo_server, optarg, 255);
			break;
			case 'm':
				strncpy(message,optarg,255);
			break;
			case 'h':
			case '?':
			default:
				printf("usage: echo -g (start) [-t<time>] <message> -p <port> -s<server url> -m<message>\n");
				goto end_cmd_echo;
		}
	}
	
	if(go)
	{
		if (echo_client_on)
		{
			printf("echo_client is working\n");
			goto end_cmd_echo;
		}
		if (xTaskCreate( tcpecho_client_handler, "echo_client", configMINIMAL_STACK_SIZE, 
			(void *)message, tskIDLE_PRIORITY + 1, &hTcpEchoClientTask )==pdPASS)
			echo_client_on =1;
		else
			printf("start echo client fail\n");
	}
	else
		printf("Echo server %s, port %d, time %d, message %s",echo_server, echo_port,echo_time,message);
end_cmd_echo:
	return;
}
    
//
// tcpecho_server_init
//
int tcpecho_server_init(void)
{
	if (xTaskCreate( tcpecho_server_handler, "echo_server", configMINIMAL_STACK_SIZE, 
		(void *)NULL, tskIDLE_PRIORITY + 1, &hTcpEchoServerTask )==pdPASS)
		printf("initial TCP echo server %d\n",ECHO_PORT);
	else
		printf("initial TCP echo server fail\n");
	return 0;
}

#endif /* LWIP_IPV4*/
