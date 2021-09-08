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
#include "sha1.h"
#include "cmd.h"

/**
  * @brief  calculate sha1 digest of memory contain.
  * @param  start address of data in memory
  * @param  length of data
  * @param  digest 
  * @retval None
  */
void sha1sum(void *data, int len, unsigned char digest[20])
{
	mbedtls_sha1_context ctx;
	
	mbedtls_sha1_init( &ctx );
	mbedtls_sha1_starts( &ctx );
	mbedtls_sha1_update( &ctx, (unsigned char *)data, len);
	mbedtls_sha1_finish( &ctx, digest );
	mbedtls_sha1_free( &ctx );
}

//=============================================================================
//  function: cmd_sha1
//      calculate file'd sha1 hash digest
//  parameters
//      argc:   1
//      argv:   filename
//=============================================================================
void cmd_sha1(int argc, char* argv[])
{
	unsigned char c[16];
    int i, fd;
    int bytes;
    unsigned char data[1024];
	mbedtls_sha1_context ctx;
	unsigned char sha1sum[20];
		
	if (argc > 1)
	{
		fd = fs_open(( const TCHAR * )argv[1], FA_READ);
		if (fd<0)
		{
        	printf ("%s can't be opened.\n", argv[1]);
        	return;
    	}
    	mbedtls_sha1_init( &ctx );
		mbedtls_sha1_starts( &ctx );
		
	    while ((bytes = fs_read (fd, data, 1024)) > 0)
    	    mbedtls_sha1_update( &ctx, (unsigned char *)data, bytes);
    	
		mbedtls_sha1_finish( &ctx, sha1sum );
		
		printf("\rHASH digest of %s\n", argv[1]);
		for (i = 0; i < 20; i++) 
    		printf("%02x", sha1sum[i]);
		mbedtls_sha1_free( &ctx );
		printf (" %s\n", argv[1]);
    	fs_close (fd);
	}
}