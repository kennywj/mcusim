#include <unistd.h>
#include "lwipopts.h"

#if LWIP_IPV4

#include <lwip/sockets.h> 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmd.h"



static xTaskHandle hTcpEchoServerTask;
//
// tcp echo server 
//
void tcpecho_server_handler(void *arg)
{
	unsigned short port = *(unsigned short *)arg;
	int socket_fd=-1,accept_fd=-1;
	int addr_size,sent_data; 
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
	sa.sin_port = htons(port);


	/* Bind the TCP socket to the port SENDER_PORT_NUM and to the current
	* machines IP address (Its defined by SENDER_IP_ADDR). 
	* Once bind is successful for UDP sockets application can operate
	* on the socket descriptor for sending or receiving data.
	*/
	if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
	{
		printf("Bind to Port Number %d failed\n",SENDER_PORT_NUM);
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
			if(recv_data < 0)
			{
				printf("receive data error %d\n",recv_data);
				break;
			}
			snprintf(send_buff,255,"Server get %d \"%s\"\n",recv_data,data_buffer);
			sent_data = send(accept_fd, send_buffer,strlen(send_buff));
			if(sent_data < 0 )
			{
				printf("send failed\n");
				break;
			}
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


//  function: cmd_echo
//      send message to echo server and get reply from server
//  parameters
//      argc:   2
//      argv:   echo <message>
//
void cmd_echo(int argc, char* argv[])
{
    
}
    
//
// tcpecho_server_init
//
int tcpecho_server_init(unsigned short port)
{
	xTaskCreate( tcpecho_server_handler, "TcpEchoServer", configMINIMAL_STACK_SIZE, 
		(void *)&port, tskIDLE_PRIORITY + 1, &hTcpEchoServerTask );
	printf("initial TCP echo server %d\n",port);
}

#endif /* LWIP_IPV4*/
