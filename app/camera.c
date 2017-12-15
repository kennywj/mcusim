#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sys.h"
#include "cmd.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSerial.h"

static char camera_on = 0, camera_exit = 0;
static int iSerialReceive = 0;
static char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
//static xQueueHandle xSerialRxQueue;
static struct _queue_ xSerialRxQueue;
static xTaskHandle hSerialTask;
static int baudid=4;	// 921600 as default
static xSemaphoreHandle camera_sem =	NULL;
static xSemaphoreHandle camera_rx_sem =	NULL;
static xSemaphoreHandle camera_tx_sem =	NULL;
//
// interrupt handler function, to receive incoming data
//
void CameraRxISR( int iFileDescriptor, void *pContext )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	ssize_t iReadResult = -1, len;
	unsigned char ucRx[256];

	if ( NULL != pContext )
	{
		do{
			iReadResult = read( iFileDescriptor, ucRx, 256 );
			if (iReadResult<=0)
				break;
			len = queue_put((struct _queue_ *)pContext, ucRx, iReadResult);
			if (len != iReadResult)
				break;
        }while(1);
        xSemaphoreGiveFromISR(camera_rx_sem, &xHigherPriorityTaskWoken);
	}
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

//
//  function: camera_rx_process
//      main function to process the recv characters from uart device
//  parametersxSemaphoreGiveFromISR
//      argc:   1
//      argv:   none
//
static void camera_rx_process( void *pvParameters )
{
    int len;
    char buf[1024];
    struct _queue_ *rxq = (struct _queue_ *)pvParameters;
    
	while(1)
	{
		xSemaphoreTake(camera_rx_sem, portMAX_DELAY);
		if (camera_exit)
			break;
		len = queue_get(rxq, buf, 1024);
		if (len)
			printf("%s:%s\n",__FUNCTION__, buf);
	}
    /* Port wasn't opened. */
    printf( "%s Task exiting.\n",__FUNCTION__ );
    hSerialTask = NULL;
    xSemaphoreGive(camera_sem);
    vTaskDelete( NULL );
}

//
//  function: camera_cmd
//      write command to camera via uart port
//      parameter:
//          char *cmd: command string
//
void sent_cmd(char *data)
{
    int ret, size;
    if (iSerialReceive<=0 )
    {
        printf("%s: device not ready\n",__FUNCTION__);
        return;
    }
    // obtain mutex
    xSemaphoreTake(camera_tx_sem, portMAX_DELAY);
    size = strlen(data);
    // transmit
    dump_frame(data,size,"%s:%d uart tx len=%d\n",__FUNCTION__,__LINE__, size);
    ret = write(iSerialReceive, data, size);
    if (ret != size)
    {
        if (ret == -1)
            printf("%s: write error %d\n",__FUNCTION__,errno);
        else
            printf("%s: ret=%d, lost %d\n",__FUNCTION__,ret, size-ret);
    }
    xSemaphoreGive(camera_tx_sem);
}


//
//  function: cmd_camera
//      diaplay and program local device ip address
//  parameters
//      argc:   2
//      argv:   -r<report time, unit seconds, 0 disable)
//
void cmd_camera(int argc, char* argv[])
{
    int c;
    int i;
    char buf[64];
    
    while((c=getopt(argc, argv, "b:d:x:h?")) != -1)
    {
        switch(c)
        {
		case 'x':
			if (camera_on)
			{
				snprintf(buf,63,"%s\r\n",optarg);
				sent_cmd(buf);
			}
			break;
        case 'd':
            strncpy(devname, optarg, DEVICE_NAME_LEN);
            printf("set device %s\n",devname);
            break;
        case 'b':
            for(i=0; i<MAX_BAUD_NUM; i++)
            {
                if (strcmp(optarg,baudstr[i])==0)
                {
                    baudid = i;
                    printf("set baudrate %s\n",baudstr[baudid]);
                    break;
                }
            }
            break;
        case 'h':
        case '?':
			printf("camera %s, device %s, baudrate %s\n",(camera_on?"":""),
				devname,baudstr[baudid]);
			printf("Usage: %s\n",curr_cmd->usage);
			break;
        default:
            printf("wrong command!\n usgae: %s\n",curr_cmd->usage);
            return;
        }
    }   // end while
    
    for(;optind<argc;optind++)
    {
		if (strcmp("on",argv[optind])==0)
		{
			// open uart device
			if (camera_on==0 &&
				pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
			{
				camera_tx_sem = xSemaphoreCreateMutex();
				camera_rx_sem = xSemaphoreCreateBinary();
				camera_sem = xSemaphoreCreateBinary();
				//xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
				queue_init(&xSerialRxQueue,0x8000);
				(void)lAsyncIORegisterCallback(iSerialReceive, CameraRxISR,
												(void *)&xSerialRxQueue );
				printf("Open device %s success\n",devname);
				xTaskCreate( camera_rx_process, "camera", 4096, (void *)&xSerialRxQueue,
                     tskIDLE_PRIORITY + 3, &hSerialTask );
				printf("start camera\n");
				camera_on = 1;
				camera_exit = 0;
			}
			else
				printf("device already actived\n");
		}
		else if (camera_on==1 && strcmp("off",argv[optind])==0)
		{
			printf("stop camera\n");
			sent_cmd("setm -m0\r\n");
			// terminate ppp session
			printf("wait camera terminate ...\n");
			camera_exit =1;
			xSemaphoreGive(camera_rx_sem);
			if (xSemaphoreTake(camera_sem, 3000)!= pdTRUE)	
				printf("wait ppp terminate fail\n");
			printf("Close %s!",devname);
			lAsyncIOSerialClose(iSerialReceive);
			iSerialReceive = 0;
			queue_exit(&xSerialRxQueue);
			//vQueueDelete(xSerialRxQueue);
			vSemaphoreDelete(camera_sem);
			vSemaphoreDelete(camera_rx_sem);
			vSemaphoreDelete(camera_tx_sem);
			camera_tx_sem = camera_rx_sem = camera_sem=NULL;
			camera_on = 0;
		}
		else if (strcmp("getpic",argv[optind])==0)
		{
			if (camera_on)
				sent_cmd("getpic\r\n");
			else
				printf("camera not enable\n");
		}
	}
    printf("camera %s, device %s, baudrate %s\n",(camera_on?"on":"off"),
		devname,baudstr[baudid]);
}
