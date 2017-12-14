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

static char camera_on = 0;
static int iSerialReceive = 0;
static char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
static xQueueHandle xSerialRxQueue;
static xTaskHandle hSerialTask;
static int baudid=4;

//
//  function: camera_rx_process
//      main function to process the recv characters from uart device
//  parameters
//      argc:   1
//      argv:   none
//
static void camera_rx_process( void *pvParameters )
{
    xQueueHandle hSerialRxQueue = ( xQueueHandle )pvParameters;
    unsigned char ch,cmd;
    unsigned char buf[MAX_PKT_SIZE], resp[MAX_MSG_SIZE];
    unsigned short off=0, len, crc, msgcrc;
    int ret;
    
    if ( NULL != hSerialRxQueue )
    {
        for ( ;; )
        {
            if ( pdFALSE == xQueueReceive( hSerialRxQueue, &ch, 20 ) )  // wait more the 20ms, expired
            {
                
                continue;
            }
        }
    }
end_uart_rx_process:
    /* Port wasn't opened. */
    printf( "%s Task exiting.\n",__FUNCTION__ );
    hSerialTask = NULL;
    vTaskDelete( NULL );
}

//
//  function: camera_cmd
//      write command to camera via uart port
//      parameter:
//          char *cmd: command string
//
void camera_cmd(char *data)
{
    int ret, size;
    if (iSerialReceive<=0 )
    {
        printf("%s: device not ready\n",__FUNCTION__);
        return;
    }
    // obtain mutex
    xSemaphoreTake(uart_tx_sem, portMAX_DELAY);
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
    xSemaphoreGive(uart_tx_sem);
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
    
    while((c=getopt(argc, argv, "b:c:o:m:f:")) != -1)
    {
        switch(c)
        {
		case 'u':
			strncpy(PPP_User,optarg,255);
			printf("set username %s\n",PPP_User);
			break;
	    case 'p':
			strncpy(PPP_Pass,optarg,255);
			printf("set password %s\n",PPP_Pass);
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
        case 'm':
            ppp_type = atoi(optarg)&0x01;
            printf("set work mode PPP %s\n",ppp_type?"server":"client");
            break;
        case 'c':
        default:
            printf("wrong command!\n usgae: %s\n",curr_cmd->usage);
            return;
        }
    }   // end while
    
    for(i=0;i<optind;i++,optind++)
    {
		if (strcmp("on",argv[optind])==0)
		{
			// open uart device
			if (camera_on==0 &&
				pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
			{
				xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
				(void)lAsyncIORegisterCallback(iSerialReceive, vAsyncSerialIODataAvailableISR,
												xSerialRxQueue );
				printf("Open device %s success\n",devname);
				xTaskCreate( camera_rx_process, "camera", 4096, xSerialRxQueue,
                     tskIDLE_PRIORITY + 3, &hSerialTask );
				printf("start camera\n");
				camera_on = 1;
			}
			else
				printf("device already actived\n");
		}
		else if (strcmp("off",argv[optind])==0)
		{
			printf("stop camera\n");
			camera_cmd("setm -m0\r\n");
			camera_on = 0;
		}
		else if (strcmp("getpic",argv[optind])==0)
		{
			if (camera_on)
				camera_cmd("getpic\r\n");
			else
				printf("camera not enable\n");
		}
	}
    
}
