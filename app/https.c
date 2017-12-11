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


//
// HTTP server initial function
//
int http_server_init()
{
	
	return 0;
}

#endif /* LWIP_IPV4*/
