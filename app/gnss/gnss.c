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
#include "cmd.h"
#include "sys.h"
#include "minmea.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSerial.h"

static int gnss_on=0;
static int exit_gnss = 0;
static char devname[DEVICE_NAME_LEN+1]="/dev/ttyUSB0";
static int iSerialReceive = 0;
static xTaskHandle hSerialTask;
static int baudid=3;	// default 115200
static xQueueHandle xSerialRxQueue;
static xSemaphoreHandle gnss_sem =	NULL;

static uint16_t gnss_data_updated = 0; // bit map to identify data updated.
static struct minmea_sentence_gga gnss_gga;
static struct minmea_sentence_rmc gnss_rmc;
static struct minmea_sentence_gll gnss_gll;
static struct minmea_sentence_gsa gnss_gsa;
static struct minmea_sentence_gst gnss_gst;
static struct minmea_sentence_gsv gnss_gsv;
static struct minmea_sentence_vtg gnss_vtg;
static struct minmea_sentence_zda gnss_zda;

/**
  * @brief       send_out
  * @param       message buffer pointer
  * @param       message size
  * @return      length of sent characters
  */
int send_out(char *buf, int len)
{
	return write(iSerialReceive, buf, len);
}
/**
  * @brief       readline
  * @param       channel
  * @param       recive buffer pointer
  * @param       buffer size
  * @return      length of receive characters
  */
static int readline(xQueueHandle hSerialRxQueue , char *buffer, int size)
{
    char ch;
    char *start = buffer;
    char *p = start;

    for (;;)
    {
    	// repeat read data and use timeout to exit
        if ( pdFALSE == xQueueReceive( hSerialRxQueue, (unsigned int *)&ch, 20 ) )  // wait more the 20ms, expired
        {
            if (exit_gnss)
                return -1;
            continue;
        }
        switch (ch)
        {
        case '\n':
            *p++ = '\n';
            *p = '\0';
            return (int)(p - start);

        case '\0':
            continue;

        default: /* including 'r' */
            *p++ = ch;
            if((p - start) >= size)
            {
                printf("ERR: data size over buffer size\n");
                return (int)(p - start);
            }

        }//end switch
    }//end for
}

//
//  function: gnss_receive_handler
//      act as a GNSS receiver and parsing GNSS message
//  parameters
//      argc:   1
//      argv:   none
//
void gnss_receive_handler( void *pvParameters )
{
    xQueueHandle hSerialRxQueue = ( xQueueHandle )pvParameters;
    char line[MINMEA_MAX_LENGTH+1]={0};
    int ret;
    
    printf("GNSS start\n");
    while (1) 
    {
        ret = readline(hSerialRxQueue, line, MINMEA_MAX_LENGTH);
    	if (ret < 0)
    		break;
        printf("%s", line);
        switch (minmea_sentence_id(line, false))
        {
	        case MINMEA_SENTENCE_RMC:
	            if (minmea_parse_rmc(&gnss_rmc, line))
	            	gnss_data_updated |= (1<<MINMEA_SENTENCE_RMC);
	        break;
	        case MINMEA_SENTENCE_GGA:
	            if (minmea_parse_gga(&gnss_gga, line))
	            	gnss_data_updated |= (1<<MINMEA_SENTENCE_GGA);
	        break;
	        case MINMEA_SENTENCE_GSA:
	        	if (minmea_parse_gsa(&gnss_gsa, line))
	        		gnss_data_updated |= (1<<MINMEA_SENTENCE_GSA);
			break;
			case MINMEA_SENTENCE_GLL:
				if (minmea_parse_gll(&gnss_gll, line))
	        		gnss_data_updated |= (1<<MINMEA_SENTENCE_GLL);
			break;
	        case MINMEA_SENTENCE_GST:
	            if (minmea_parse_gst(&gnss_gst, line))
	            	gnss_data_updated |= (1<<MINMEA_SENTENCE_GST);
	        break;
	        case MINMEA_SENTENCE_GSV:
	            if (minmea_parse_gsv(&gnss_gsv, line))
	            	gnss_data_updated |= (1<<MINMEA_SENTENCE_GSV);
	        break;
	        case MINMEA_SENTENCE_VTG:
	            if (minmea_parse_vtg(&gnss_vtg, line))
	            	gnss_data_updated |= (1<<MINMEA_SENTENCE_VTG);
	        break;
	        case MINMEA_SENTENCE_ZDA:
	            if (minmea_parse_zda(&gnss_zda, line))
	            	gnss_data_updated |= (1<<MINMEA_SENTENCE_ZDA);
	        break;
	        case MINMEA_INVALID:
		    break;
	        default:
		    break;
        }	// end switch
    }	// end while
    
    printf( "Close %s! %s Task exiting.\n",devname,__FUNCTION__ );
    exit_gnss=0;
    hSerialTask = NULL;
    xSemaphoreGive(gnss_sem);
    vTaskDelete( NULL );    
}

//
//  function: gnss_control
//      GNSS module control commands
//  parameters
//      1: enable receive GNSS message, 0: disable receive GNSS message
//
int gnss_control(int action)
{
	if (gnss_on==1 && action==0)
	{
		// terminate ppp session
		exit_gnss = 1;
		printf("wait GNSS terminate ...\n");
		if (xSemaphoreTake(gnss_sem, 30000)!= pdTRUE)	
			printf("wait GNSS terminate fail\n");
		printf("Close %s!",devname);
		lAsyncIOSerialClose(iSerialReceive);
        iSerialReceive = 0;
        vQueueDelete(xSerialRxQueue);
        gnss_on = 0;
        return 0;
	}
	else if (gnss_on==0 && action == 1)
	{
		// initial ppp session
		if (hSerialTask!=NULL)
		{
			printf("GNSS module working\n");
			return -2;
		}
		if (gnss_sem == NULL)
			vSemaphoreCreateBinary(gnss_sem);
		
		if (gnss_sem)
			xSemaphoreTake(gnss_sem,0);
		// open uart device
		if ( pdTRUE == lAsyncIOSerialOpen( devname, &iSerialReceive,  baudrate[baudid]) )
		{
			xSerialRxQueue = xQueueCreate( MAX_PKT_SIZE*2, sizeof ( unsigned char ) );
			(void)lAsyncIORegisterCallback( iSerialReceive, vAsyncSerialIODataAvailableISR,
                                        xSerialRxQueue );
			printf("Open device %s success\n",devname);
			xTaskCreate( gnss_receive_handler, "GNSS", 4096, xSerialRxQueue,
                     tskIDLE_PRIORITY + 4, &hSerialTask );                    
			gnss_on = 1;
		}
		else
			printf("Open %s fail\n",devname);
		return 0;
	}
	else
		return -1;
}


/**
  * @brief       stop Teseo_liv2f GNSS module
  * @param       none
  * @return      1: active, 0: deactive.
  */
int gnss_status()
{
	return gnss_on;
}

/**
  * @brief       get data from Teseo_liv2f GNSS module 
  * @param       Teseo message ID
  * @param       size of data buffer
  * @param       data buffer pointer
  * @return      1: got data, 0: not got data
  */
int gnss_get(int id, int bufsize, uint8_t *data)
{
	int ret = 0, len;
	
	if (!gnss_on)
		return ret;
		
	if (gnss_data_updated & (1<<id))
	{
		ret = 1;
		switch(id)
		{
			case MINMEA_SENTENCE_RMC:
				len = (sizeof(struct minmea_sentence_rmc)>bufsize?bufsize:sizeof(struct minmea_sentence_rmc));
				memcpy(data,&gnss_rmc,len);
			break;
    		case MINMEA_SENTENCE_GGA:
				len = (sizeof(struct minmea_sentence_gga)>bufsize?bufsize:sizeof(struct minmea_sentence_gga));
				memcpy(data,&gnss_gga,len);
			break;
    		case MINMEA_SENTENCE_GSA:
				len = (sizeof(struct minmea_sentence_gsa)>bufsize?bufsize:sizeof(struct minmea_sentence_gsa));
				memcpy(data,&gnss_gsa,len);
			break;
    		case MINMEA_SENTENCE_GLL:
				len = (sizeof(struct minmea_sentence_gll)>bufsize?bufsize:sizeof(struct minmea_sentence_gll));
				memcpy(data,&gnss_gll,len);
			break;
    		case MINMEA_SENTENCE_GST:
				len = (sizeof(struct minmea_sentence_gst)>bufsize?bufsize:sizeof(struct minmea_sentence_gst));
				memcpy(data,&gnss_gst,len);
			break;
    		case MINMEA_SENTENCE_GSV:
				len = (sizeof(struct minmea_sentence_gsv)>bufsize?bufsize:sizeof(struct minmea_sentence_gsv));
				memcpy(data,&gnss_gsv,len);
			break;
    		case MINMEA_SENTENCE_VTG:
				len = (sizeof(struct minmea_sentence_vtg)>bufsize?bufsize:sizeof(struct minmea_sentence_vtg));
				memcpy(data,&gnss_vtg,len);
			break;
    		case MINMEA_SENTENCE_ZDA:
				len = (sizeof(struct minmea_sentence_zda)>bufsize?bufsize:sizeof(struct minmea_sentence_zda));
				memcpy(data,&gnss_zda,len);
			break;
			default:
				ret = 0;
			break;
		}
	}
	return ret;
}

//
//  function: cmd_gnss
//      gnss module control commands
//  parameters
//      argc:
//      argv:
//
void cmd_gnss(int argc, char* argv[])
{
    int c;
    int i;

    while((c=getopt(argc, argv, "d:b:")) != -1)
    {
        switch(c)
        {
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
        default:
            printf("wrong command!\n usgae: %s\n",curr_cmd->usage);
            return;
        }
    }   // end while
    
    for(;optind<argc;optind++)
	{
		if (strcmp(argv[optind],"on")==0)
			gnss_control(1);
		else if (strcmp(argv[optind],"off")==0)
			gnss_control(0);
	}
	
	if (gnss_status())
    {
    	typedef union 
		{
			struct minmea_sentence_gga gga;
			struct minmea_sentence_rmc rmc;
			struct minmea_sentence_gsa gsa;
			struct minmea_sentence_gll gll;
			struct minmea_sentence_gst gst;
			struct minmea_sentence_gsv gsv;
			struct minmea_sentence_vtg vtg;
			struct minmea_sentence_zda zda;
		} gnss_data_t;
    	
    	gnss_data_t data;
    	int i,j;
    	printf("GNSS module enabled\n");
    	for (i=MINMEA_SENTENCE_RMC; i<=MINMEA_SENTENCE_ZDA;i++)
    	{
    		if (gnss_get(i, sizeof(gnss_data_t), (uint8_t *)&data))
    		{
    			switch(i)
    			{
    				case MINMEA_SENTENCE_GGA:
		                printf("$xxGGA: fix quality: %d\n", data.gga.fix_quality);
    				break;
    				case MINMEA_SENTENCE_RMC:
    					printf("$xxRMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
	                       (int)data.rmc.latitude.value, (int)data.rmc.latitude.scale,
	                       (int)data.rmc.longitude.value, (int)data.rmc.longitude.scale,
	                       (int)data.rmc.speed.value, (int)data.rmc.speed.scale);
	                	printf("       fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
	                       (int)minmea_rescale(&data.rmc.latitude, 1000),
	                       (int)minmea_rescale(&data.rmc.longitude, 1000),
	                       (int)minmea_rescale(&data.rmc.speed, 1000));
	                	printf("       floating point degree coordinates and speed: (%f,%f) %f\n",
	                       minmea_tocoord(&data.rmc.latitude),
	                       minmea_tocoord(&data.rmc.longitude),
	                       minmea_tofloat(&data.rmc.speed));
    				break;
    				case MINMEA_SENTENCE_GSA:
    					printf("$xxGSA: %d,%d,%d\n",
	                       (int)minmea_rescale(&data.gsa.pdop, 100),
	                       (int)minmea_rescale(&data.gsa.hdop, 100),
	                       (int)minmea_rescale(&data.gsa.vdop, 100));
    				break;
    				case MINMEA_SENTENCE_GLL:
    					printf("$xxGLL: Geographic Positioning latitude,longitude (%d,%d)\n",
	                       (int)minmea_rescale(&data.gll.latitude, 1000),
	                       (int)minmea_rescale(&data.gll.longitude, 1000));
	                    printf("        %d:%d:%d Status %c, Mode %c\n",
	                       data.gll.time.hours,
	                       data.gll.time.minutes,
	                       data.gll.time.seconds,
	                       data.gll.status,
	                       data.gll.mode);
    				break;
    				case MINMEA_SENTENCE_GST:
	    				printf("$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)\n",
	                       (int)data.gst.latitude_error_deviation.value, (int)data.gst.latitude_error_deviation.scale,
	                       (int) data.gst.longitude_error_deviation.value, (int)data.gst.longitude_error_deviation.scale,
	                       (int)data.gst.altitude_error_deviation.value, (int)data.gst.altitude_error_deviation.scale);
	                	printf("       fixed point latitude,longitude and altitude error deviation"
	                       " scaled to one decimal place: (%d,%d,%d)\n",
	                       (int)minmea_rescale(&data.gst.latitude_error_deviation, 10),
	                       (int)minmea_rescale(&data.gst.longitude_error_deviation, 10),
	                       (int)minmea_rescale(&data.gst.altitude_error_deviation, 10));
	                	printf("       floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
	                       minmea_tofloat(&data.gst.latitude_error_deviation),
	                       minmea_tofloat(&data.gst.longitude_error_deviation),
	                       minmea_tofloat(&data.gst.altitude_error_deviation));
    				break;
    				case MINMEA_SENTENCE_GSV:
    					printf("$xxGSV: message %d of %d\n", data.gsv.msg_nr, data.gsv.total_msgs);
	                	printf("        sattelites in view: %d\n", data.gsv.total_sats);
	                	for (j = 0; j < 4; j++)
	                	{
	                    	printf("       sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
	                           data.gsv.sats[j].nr,
	                           data.gsv.sats[j].elevation,
	                           data.gsv.sats[j].azimuth,
	                           data.gsv.sats[j].snr);
	                    }
    				break;
    				case MINMEA_SENTENCE_VTG:
    					printf("$xxVTG: true track degrees = %f\n",
	                       minmea_tofloat(&data.vtg.true_track_degrees));
	                	printf("        magnetic track degrees = %f\n",
	                       minmea_tofloat(&data.vtg.magnetic_track_degrees));
	                	printf("        speed knots = %f\n",
	                       minmea_tofloat(&data.vtg.speed_knots));
	                	printf("        speed kph = %f\n",
	                       minmea_tofloat(&data.vtg.speed_kph));
    				break;
    				case MINMEA_SENTENCE_ZDA:
    					printf("$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n",
	                       data.zda.time.hours,
	                       data.zda.time.minutes,
	                       data.zda.time.seconds,
	                       data.zda.date.day,
	                       data.zda.date.month,
	                       data.zda.date.year,
	                       data.zda.hour_offset,
	                       data.zda.minute_offset);
    				break;
    			}
    		}
    	}
    }
    else
    	printf("GNSS module not enable\n");
}
