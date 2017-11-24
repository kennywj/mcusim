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
