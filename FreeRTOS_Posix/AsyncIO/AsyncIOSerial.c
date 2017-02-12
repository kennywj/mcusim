/*
 * AsyncIOSerial.c
 *
 *  Created on: 9 Apr 2010
 *      Author: William Davy
 */
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "AsyncIOSerial.h"
/*---------------------------------------------------------------------------*/

/* See 'man termios' for more details on configuring the serial port. */
long lAsyncIOSerialOpen( const char *pcDevice, int *piDeviceDescriptor , int baud)
{
int iSerialDevice = 0;
struct termios xTerminalSettings;
long lReturn = pdFALSE;

	/* Open the port that was passed in. */
	iSerialDevice = open( pcDevice, O_RDWR | O_NOCTTY | O_NONBLOCK );
	if ( iSerialDevice < 0 )
	{
	    printf("Open device %s fail, errno=%d\n",pcDevice, errno);
		lReturn = pdFALSE;
	}
	else
	{
		/* Grab a copy of the current terminal settings. */
		if ( 0 == tcgetattr( iSerialDevice, &xTerminalSettings ) )
		{
			/* Configure the port to the Hard-coded values. */
			cfmakeraw( &xTerminalSettings );
			cfsetspeed( &xTerminalSettings, baud );
			
			//xTerminalSettings.c_lflag &= ~(ICANON|ECHO);  /* Clear ICANON and ECHO. */
            //xTerminalSettings.c_lflag |= TCSANOW;         /* immediate response */
            xTerminalSettings.c_cc[VMIN] = 1;
            xTerminalSettings.c_cc[VTIME] = 0;
			/* Set the terminal settings. */
			lReturn = ( 0 == tcsetattr(iSerialDevice, TCSANOW, &xTerminalSettings) );

			/* Pass out the device descriptor for subsequent calls to AsyncIORegisterCallback() */
			*piDeviceDescriptor = iSerialDevice;
		}
	}

	return lReturn;
}
 /*---------------------------------------------------------------------------*/

/* Define a callback function which is called when data is available. */
void vAsyncSerialIODataAvailableISR( int iFileDescriptor, void *pContext )
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
ssize_t iReadResult = -1;
unsigned char ucRx;
    do{
        iReadResult = read( iFileDescriptor, &ucRx, 1 );
        if (iReadResult!=1)
            break;
        //printf("%x",ucRx);    
        if ( NULL != pContext )
        {        
			if ( pdTRUE != xQueueSendFromISR( (xQueueHandle)pContext, &ucRx, &xHigherPriorityTaskWoken ) )
            {
				// the queue is full.
				printf("queue full drop %x(%c)\n",ucRx,ucRx);
			}
        }
    }while(1);
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
/*
	// This handler only processes a single byte/character at a time.
	iReadResult = read( iFileDescriptor, &ucRx, 1 );
	if ( 1 == iReadResult )
	{
	    printf("%x",ucRx);
		if ( NULL != pContext )
		{
			// Send the received byte to the queue.
			if ( pdTRUE != xQueueSendFromISR( (xQueueHandle)pContext, &ucRx, &xHigherPriorityTaskWoken ) )
			{
				// the queue is full.
				printf("queue full drop %x(%c)\n",ucRx,ucRx);
			}
		}
	}
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
*/	
}
/*---------------------------------------------------------------------------*/
