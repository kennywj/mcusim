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
#include "ff.h"
#include "fs_port.h"
#include "cmd.h"

#define RECV_TIMEOUT 5000
#define RECV_RETRIES 5

#define TFTPS_PORT	69
#define TFTPS_PATH "/"


/* tftp opcode mnemonic */
enum opcode {
     RRQ=1,
     WRQ,
     DATA,
     ACK,
     ERROR
};

/* tftp transfer mode */
enum mode {
     NETASCII=1,
     OCTET
};

/* tftp message structure */
typedef union {

     uint16_t opcode;

     struct {
          uint16_t opcode; /* RRQ or WRQ */             
          uint8_t filename_and_mode[514];
     } request;     

     struct {
          uint16_t opcode; /* DATA */
          uint16_t block_number;
          uint8_t data[512];
     } data;

     struct {
          uint16_t opcode; /* ACK */             
          uint16_t block_number;
     } ack;

     struct {
          uint16_t opcode; /* ERROR */     
          uint16_t error_code;
          uint8_t error_string[512];
     } error;

} tftp_message;

static xTaskHandle hTftpsTask;
static char base_directory[256] = TFTPS_PATH;

ssize_t tftp_send_data(int s, uint16_t block_number, uint8_t *data,
                       ssize_t dlen, struct sockaddr_in *sock, socklen_t slen)
{
     tftp_message m;
     ssize_t c;

     m.opcode = htons(DATA);
     m.data.block_number = htons(block_number);
     memcpy(m.data.data, data, dlen);

     if ((c = sendto(s, &m, 4 + dlen, 0,
                     (struct sockaddr *) sock, slen)) < 0) {
          printf("server: sendto()");
     }

     return c;
}


ssize_t tftp_send_ack(int s, uint16_t block_number,
                      struct sockaddr_in *sock, socklen_t slen)
{
     tftp_message m;
     ssize_t c;

     m.opcode = htons(ACK);
     m.ack.block_number = htons(block_number);

     if ((c = sendto(s, &m, sizeof(m.ack), 0,
                     (struct sockaddr *) sock, slen)) < 0) {
          printf("server: sendto()");
     }

     return c;
}

ssize_t tftp_send_error(int s, int error_code, char *error_string,
                        struct sockaddr_in *sock, socklen_t slen)
{
     tftp_message m;
     ssize_t c;

     if(strlen(error_string) >= 512) {
          printf("server: tftp_send_error(): error string too long\n");
          return -1;
     }

     m.opcode = htons(ERROR);
     m.error.error_code = error_code;
     strcpy(m.error.error_string, error_string);

     if ((c = sendto(s, &m, 4 + strlen(error_string) + 1, 0,
                     (struct sockaddr *) sock, slen)) < 0) {
          printf("server: sendto()");
     }

     return c;
}

ssize_t tftp_recv_message(int s, tftp_message *m, struct sockaddr_in *sock, socklen_t *slen)
{
     ssize_t c;

     if ((c = recvfrom(s, m, sizeof(*m), 0, (struct sockaddr *) sock, slen)) < 0
          && errno != EAGAIN) {
          printf("%s: recvfrom() %c\n",__FUNCTION__,c);
     }

     return c;
}


void tftp_handle_request(tftp_message *m, ssize_t len,
                         struct sockaddr_in *client_sock, socklen_t slen)
{
     int s=-1, fd=-1;
     int timeout = RECV_TIMEOUT;
     char *filename, *mode_s, *end;
     int mode;
     uint16_t opcode;

     /* open new socket, on new port, to handle client request */
     if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
          printf("server: socket()");
          goto end_tftp_handle_request;
     }

     if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
          printf("server: setsockopt()");
          goto end_tftp_handle_request;
     }

     /* parse client request */

     filename = m->request.filename_and_mode;
     end = &filename[len - 2 - 1];

     if (*end != '\0') {
          printf("%s.%u: invalid filename or mode\n",
                 inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
          tftp_send_error(s, 0, "invalid filename or mode", client_sock, slen);
          goto end_tftp_handle_request;
     }

     mode_s = strchr(filename, '\0') + 1; 

     if (mode_s > end) {
          printf("%s.%u: transfer mode not specified\n",
                 inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
          tftp_send_error(s, 0, "transfer mode not specified", client_sock, slen);
          goto end_tftp_handle_request;
     }

     if(strncmp(filename, "../", 3) == 0 || strstr(filename, "/../") != NULL ||
        (filename[0] == '/' && strncmp(filename, base_directory, strlen(base_directory)) != 0)) {
          printf("%s.%u: filename outside base directory\n",
                 inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
          tftp_send_error(s, 0, "filename outside base directory", client_sock, slen);
          goto end_tftp_handle_request;
     }

     opcode = ntohs(m->opcode);
     
     fd = fs_open(filename, opcode == RRQ ? FA_READ : (FA_CREATE_NEW| FA_WRITE)); 
     if (fd < 0) {
			printf("server: fopen()");
			tftp_send_error(s, errno, strerror(errno), client_sock, slen);
			goto end_tftp_handle_request;
     }

     mode = strcasecmp(mode_s, "netascii") ? NETASCII :
          strcasecmp(mode_s, "octet")    ? OCTET    :
          0;

     if (mode == 0) {
          printf("%s.%u: invalid transfer mode\n",
                 inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
			tftp_send_error(s, 0, "invalid transfer mode", client_sock, slen);
			goto end_tftp_handle_request;
     }

     printf("%s.%u: request received: %s '%s' %s\n", 
            inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port),
            ntohs(m->opcode) == RRQ ? "get" : "put", filename, mode_s);

     // TODO: add netascii handling

     if (opcode == RRQ) {
          tftp_message m;

          uint8_t data[512];
          ssize_t dlen, c;

          uint16_t block_number = 0;
         
          int countdown;
          int to_close = 0;
         
          while (!to_close) {

               dlen = fs_read(fd, data, sizeof(data));
               if (dlen<0)
               {
					printf("read file error\n");
					break;
				}
               block_number++;
              
               if (dlen < 512) { // last data block to send
                    to_close = 1;
               }

               for (countdown = RECV_RETRIES; countdown; countdown--) {

                    c = tftp_send_data(s, block_number, data, dlen, client_sock, slen);
               
                    if (c < 0) {
                         printf("%s.%u: transfer killed\n",
                                inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                         goto end_tftp_handle_request;
                    }

                    c = tftp_recv_message(s, &m, client_sock, &slen);

                    if (c >= 0 && c < 4) {
                         printf("%s.%u: message with invalid size received\n",
                                inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                         tftp_send_error(s, 0, "invalid request size", client_sock, slen);
                         goto end_tftp_handle_request;
                    }

                    if (c >= 4) {
                         break;
                    }

                    if (errno != EAGAIN) {
                         printf("%s.%u: transfer killed\n",
                                inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                         goto end_tftp_handle_request;
                    }

               }

               if (!countdown) {
                    printf("%s.%u: transfer timed out\n",
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                    goto end_tftp_handle_request;
               }

               if (ntohs(m.opcode) == ERROR)  {
                    printf("%s.%u: error message received: %u %s\n",
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port),
                           ntohs(m.error.error_code), m.error.error_string);
                    goto end_tftp_handle_request;
               }

               if (ntohs(m.opcode) != ACK)  {
                    printf("%s.%u: invalid message during transfer received\n",
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                    tftp_send_error(s, 0, "invalid message during transfer", client_sock, slen);
                    goto end_tftp_handle_request;
               }
              
               if (ntohs(m.ack.block_number) != block_number) { // the ack number is too high
                    printf("%s.%u: invalid ack number received\n", 
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                    tftp_send_error(s, 0, "invalid ack number", client_sock, slen);
                    goto end_tftp_handle_request;
               }

          }

     }

     else if (opcode == WRQ) {

          tftp_message m;

          ssize_t c;

          uint16_t block_number = 0;
         
          int countdown;
          int to_close = 0;

          c = tftp_send_ack(s, block_number, client_sock, slen);
          
          if (c < 0) {
               printf("%s.%u: transfer killed\n",
                      inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
               goto end_tftp_handle_request;
          }

          while (!to_close) {

               for (countdown = RECV_RETRIES; countdown; countdown--) {

                    c = tftp_recv_message(s, &m, client_sock, &slen);

                    if (c >= 0 && c < 4) {
                         printf("%s.%u: message with invalid size received\n",
                                inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                         tftp_send_error(s, 0, "invalid request size", client_sock, slen);
                         goto end_tftp_handle_request;
                    }

                    if (c >= 4) {
                         break;
                    }

                    if (errno != EAGAIN) {
                         printf("%s.%u: transfer killed\n",
                                inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                         goto end_tftp_handle_request;
                    }

                    c = tftp_send_ack(s, block_number, client_sock, slen);
              
                    if (c < 0) {
                         printf("%s.%u: transfer killed\n",
                                inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                         goto end_tftp_handle_request;
                    }

               }

               if (!countdown) {
                    printf("%s.%u: transfer timed out\n",
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                    goto end_tftp_handle_request;
               }

               block_number++;

               if (c < sizeof(m.data)) {
                    to_close = 1;
               }

               if (ntohs(m.opcode) == ERROR)  {
                    printf("%s.%u: error message received: %u %s\n",
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port),
                           ntohs(m.error.error_code), m.error.error_string);
                    goto end_tftp_handle_request;
               }

               if (ntohs(m.opcode) != DATA)  {
                    printf("%s.%u: invalid message during transfer received\n",
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                    tftp_send_error(s, 0, "invalid message during transfer", client_sock, slen);
                    goto end_tftp_handle_request;
               }
              
               if (ntohs(m.ack.block_number) != block_number) {
                    printf("%s.%u: invalid block number received\n", 
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
                    tftp_send_error(s, 0, "invalid block number", client_sock, slen);
                    goto end_tftp_handle_request;
               }
               
			   c = fs_write(fd, m.data.data, c - 4);
               if (c < 0) {
                    printf("server: fwrite()");
                    goto end_tftp_handle_request;
               }

               c = tftp_send_ack(s, block_number, client_sock, slen);
          
               if (c < 0) {
                    printf("%s.%u: transfer killed\n",
                           inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
					goto end_tftp_handle_request;
               }

          }

     }

     printf("%s.%u: transfer completed\n",
            inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
end_tftp_handle_request:
	if (fd>0)
		fs_close(fd);
	if (s>=0)
		close(s);
	return;
}



//
//  function: tftp_server_thread
//      main function to process the recvived TFTP packet from uart device 
//  parameters
//      argc:   1
//      argv:   none
//
void tftp_server_thread( void *pvParameters )
{
	int s=-1;
	struct sockaddr_in server_sock;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
          printf("server: socket() error");
          goto end_tftp_server_thread;
     }

     server_sock.sin_family = AF_INET;
     server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
     server_sock.sin_port = htons(TFTPS_PORT);

     if (bind(s, (struct sockaddr *) &server_sock, sizeof(server_sock)) == -1) {
          printf("server: bind()");
          goto end_tftp_server_thread;
     }
     printf("tftp server: listening on %d\n", ntohs(server_sock.sin_port));

     while (1) {
          struct sockaddr_in client_sock;
          socklen_t slen = sizeof(client_sock);
          ssize_t len;
          tftp_message message;
          uint16_t opcode;

          if ((len = tftp_recv_message(s, &message, &client_sock, &slen)) < 0) {
               continue;
          }

          if (len < 4) { 
               printf("%s.%u: request with invalid size received\n",
                      inet_ntoa(client_sock.sin_addr), ntohs(client_sock.sin_port));
               tftp_send_error(s, 0, "invalid request size", &client_sock, slen);
               continue;
          }
          opcode = ntohs(message.opcode);

          if (opcode == RRQ || opcode == WRQ) 
               /* handle the request */
			tftp_handle_request(&message, len, &client_sock, slen);
          else 
          {
               printf("%s.%u: invalid request received: opcode %d\n", 
                      inet_ntoa(client_sock.sin_addr), ntohs(client_sock.sin_port),
                      opcode);
               tftp_send_error(s, 0, "invalid opcode", &client_sock, slen);
          }
     }

end_tftp_server_thread:
	if (s>=0)
		close(s);
	hTftpsTask = NULL;
	vTaskDelete( NULL ); 
}

//
// function: tftp_server_init
//	start a TFTP Server
//
int tftp_server_init()
{
	xTaskCreate( tftp_server_thread, "tftp_server", 4096, NULL, tskIDLE_PRIORITY + 4, &hTftpsTask );
	return 0;
}

#endif /* LWIP_IPV4*/
