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
#include "md5.h"
#include "cmd.h"

/**
  * @brief  calculate md5 digest of memory contain.
  * @param  start address of data in memory
  * @param  length of data
  * @param  digest 
  * @retval None
  */
void md5sum(void *data, int len, unsigned char digest[16])
{
	mbedtls_md5_context ctx;
	
	mbedtls_md5_init( &ctx );
   	mbedtls_md5_starts( &ctx );
	mbedtls_md5_update (&ctx, (unsigned char *)data, len);
	mbedtls_md5_finish (&ctx, digest);
	mbedtls_md5_free( &ctx );
}
//=============================================================================
//  function: cmd_md5
//      calculate file'd md5 sum
//  parameters
//      argc:   1
//      argv:   filename
//=============================================================================


void cmd_md5(int argc, char* argv[])
{
	unsigned char c[16];
    int i, fd;
    mbedtls_md5_context ctx;
    int bytes;
    unsigned char data[1024];

	if (argc > 1)
	{
		fd = fs_open(( const TCHAR * )argv[1], FA_READ);
		if (fd<0)
		{
        	printf ("%s can't be opened.\n", argv[1]);
        	return;
    	}
    	
		mbedtls_md5_init( &ctx );
		
    	mbedtls_md5_starts( &ctx );
    	
	    while ((bytes = fs_read (fd, data, 1024)) > 0)
    	    mbedtls_md5_update (&ctx, data, bytes);
    	    
    	mbedtls_md5_finish (&ctx, c);
    	
    	for (i = 0; i < 16; i++) 
    		printf("%02x", c[i]);
    		
    	mbedtls_md5_free( &ctx );
    	printf (" %s\n", argv[1]);
    	fs_close (fd);
	}
}