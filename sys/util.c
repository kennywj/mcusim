//
//  module: device tunnel module
//      tunnutil.h
//      active to builde up tunnel with specified server, multiplex/demutiplex traffice 
//      from server to local service or vice vera
//
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>
#include <stdarg.h> 
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "sys.h"

//
// utility of queue
//
/*-----------------------------------------------------------*/
unsigned long uxQueueSendPassedCount = 0;
void vMainQueueSendPassed( void )
{
	/* This is just an example implementation of the "queue send" trace hook. */
	uxQueueSendPassedCount++;
}
//
//  function: queue_init
//      create a queue buffer
//  parameters:
//      give structure of queue pointer
//      size of buffer
//  return:
//      SYS_OK: success
//      SYS_NOMEM: fail , no memory
//
int queue_init(struct _queue_ *q, int size)
{
	memset(q,0,sizeof(struct _queue_));
	q->buf = malloc(size);
	if (q->buf)
	{	
		q->size = size;
		q->put=q->get=q->full=0;
		//q->sem = xSemaphoreCreateMutex();
		return QUEUE_OK;
	}
	else
		return QUEUE_NOMEM_ERR;
}

//
//  function: queue_exit
//      free the queue
//  parameters:
//      give structure of queue pointer
//      size of buffer
//  return:
//      none
//
void queue_exit(struct _queue_ *q)
{
	if (q && q->buf)
	{	
		free(q->buf);
		q->buf = NULL;
		q->size = 0;
		//vSemaphoreDelete(q->sem);
		//q->sem = NULL;
	}
}

//
//  function: queue_put
//      put data into fifo, return success or queue full
//  parameters:
//      give structure of queue pointer
//      start data pointer
//      size of data
//  return:
//      >0 number of put data
//      TUNN_QUEUE_FULL: queue full
//      SYS_NOT_AVAILABLE: servic not available 
//  
int queue_put(struct _queue_ *q, char *data, int size)
{
	int i;
	
	if (!q || !q->buf)
		return QUEUE_NOTAVAIL_ERR;
		
	//if (xSemaphoreTake( q->sem, ( TickType_t ) 10 ) != pdTRUE)
	//	return QUEUE_COLLI_ERR;
		
	for(i=0;i<size;i++)
	{
		if (q->full)
		{	
			printf( "%s full\n",__FUNCTION__);	// output debug message
			return QUEUE_COLLI_ERR;
		}
		q->buf[q->put++]=data[i];
		if (q->put>=q->size)
			q->put = 0;
		if (q->put == q->get)
			q->full = 1;
	}
	//xSemaphoreGive( q->sem );
	return i;	
}


//
//  function: queue_get
//      get data from fifo, return get size
//  parameters:
//      give structure of queue pointer
//      start buffer pointer
//      size of buffer
//  return:
//      size of got data
//      SYS_NOT_AVAILABLE: queue not available
//
int queue_get(struct _queue_ *q,char *buf, int bufsize)
{
	int i;
	
	if (!q || !q->buf)
		return QUEUE_NOTAVAIL_ERR;
		
	//if (xSemaphoreTake( q->sem, ( TickType_t ) 10 ) != pdTRUE)
	//	return QUEUE_COLLI_ERR;
		
	for(i=0;i<bufsize;i++)
	{
		if (q->get == q->put && !q->full)	// empty?
			break;
		buf[i]=q->buf[q->get++];
		if (q->get >= q->size)
			q->get  = 0;
		q->full = 0;
	}
	//xSemaphoreGive( q->sem );
	return i;
}

//
//  function: queue_peek
//      get data from fifo, but not update the internal read pointer, return get size
//  parameters:
//      give structure of queue pointer
//      start buffer pointer
//      size of buffer
//  return:
//      size of got data
//      SYS_NOT_AVAILABLE: queue not available
//
int queue_peek(struct _queue_ *q,char *buf, int bufsize)
{
	int i;

	if (!q || !q->buf)
		return QUEUE_NOTAVAIL_ERR;

	//if (xSemaphoreTake( q->sem, ( TickType_t ) 10 ) != pdTRUE)
	//	return QUEUE_COLLI_ERR;

	int get = q->get;
	int full = q->full;
	for(i=0;i<bufsize;i++)
	{
		if (get == q->put && !full)	// empty?
			break;

		buf[i]=q->buf[get++];
		if (get >= q->size)
			get = 0;

		full = 0;
	}
	//xSemaphoreGive( q->sem );
	return i;
}

//
//  function: queue_move
//      move get pointer, drop first n bytes of queue data
//  parameters:
//      give structure of queue pointer
//      start buffer pointer
//      number of moved bytes
//  return:
//      number of moved bytes
//      SYS_NOT_AVAILABLE: queue not available
//
int queue_move(struct _queue_ *q, int n)
{
	if (!q || !q->buf)
		return QUEUE_NOTAVAIL_ERR;

	//if (xSemaphoreTake( q->sem, ( TickType_t ) 10 ) != pdTRUE)
	//	return QUEUE_COLLI_ERR;

	int data_size = 0;
	int ret = 0;
	if (q->full)
		data_size = q->size;
	else
		data_size = (q->put >= q->get ? (q->put - q->get) : (q->size + q->put - q->get));

	ret = (data_size > n ? n : data_size);

	q->get += ret;
	if (q->get >= q->size)
	{
		q->get -= q->size;
	}

	q->full = 0;

	//xSemaphoreGive( q->sem );
	return ret;
}

//
//  function: queue_data_size
//      get data size in fifo
//  parameters:
//      give structure of queue pointer
//  return:
//      number of moved bytes 
// 
int queue_data_size(struct _queue_ *q)
{
	int ret = 0;
	
	if (!q || !q->buf)
	    return 0;
	    
	//if (xSemaphoreTake( q->sem, ( TickType_t ) 10 ) != pdTRUE)
	//	return 0;
	if (q->full)
		ret = q->size;
	else if (q->put >= q->get)
		ret = (q->put - q->get);
	else
		ret = (q->size + q->put - q->get);
	//xSemaphoreGive( q->sem );
	return ret;
}

//
//  function: queue_space
//      get free space in queue
//  parameters:
//      give structure of queue pointer
//  return:
//      number of bytes of buffer size 
// 
int queue_space(struct _queue_ *q)
{
    if (!q || !q->buf)
	    return 0;
    return q->size - queue_data_size(q);
}

//
//  function: queue_reset
//      reset queue, ignore data in queue
//  parameters:
//      give structure of queue pointer
//  return:
//      SYS_OK: success             
//      SYS_NOT_AVAILABLE: queue not available
// 
int queue_reset(struct _queue_ *q)
{
    if (!q || !q->buf)
	    return QUEUE_NOTAVAIL_ERR;
	    
	//if (xSemaphoreTake( q->sem, ( TickType_t ) 10 ) != pdTRUE)
	//	return QUEUE_COLLI_ERR;
		
	q->put=q->get=q->full=0;	
	
	//xSemaphoreGive( q->sem );
	return SYS_OK;	
}

//
//  function: fd_set_blocking
//      set block or none-block for socket
//  parameters:
//      socket id,
//		blocking: 1: block, 0 none block
//  return:
//      SYS_OK: success             
//      SYS_NOT_AVAILABLE: queue not available
// 
int fd_set_blocking(int fd, int blocking) {
    /* Save the current flags */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return 0;

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}


// function: get_time_milisec
//      get system time ticks
//  parameters:
//      none
//  return:
//      current minisecs
//
unsigned int get_time_milisec(void)
{
    struct timespec ts;
    unsigned int tm;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    tm = (ts.tv_sec * 1000) + (unsigned int)(ts.tv_nsec / 1000000);
    return tm;
}

//
//  function: dump_frame
//      dumpe memory 
//      parameters
//          message string
//          start of memory
//          length of memory
//      return
//          0       success
//          other   fail
//
int show_type = 0; // show bytes
#define MAX_FMT_SIZE 256
void dump_frame(char *start, int len, const char * fmt, ...)
{
    unsigned short  i;
    unsigned char *p=(unsigned char *)start;
    char ch[16+1]= {0};
    va_list argp;
    
    if (len<=0)
    {
        fprintf(stderr,"size overrun %u\n",len);
        len &= 0x7fff;
    }
    va_start(argp,fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    while(len>16)
    {
        for (i=0; i<16; i++)
            ch[i] = (p[i]<0x20 || p[i]>=0x7f) ? '.' : p[i];

        //fprintf(stderr,"%08x: ", (p-(unsigned char *)frame));
        fprintf(stderr,"%08x: ", p);
        switch(show_type)
        {
            case 2:
                fprintf(stderr,"%08X %08X-%08X %08X | ",
                    *(unsigned int *)&p[0], *(unsigned int *)&p[4], *(unsigned int *)&p[8], *(unsigned int *)&p[12]);
            break;
            case 1:
                fprintf(stderr,"%04X %04X %04X %04X-%04X %04X %04X %04X | ",
                    *(unsigned short *)&p[0], *(unsigned short *)&p[2], *(unsigned short *)&p[4], 
                    *(unsigned short *)&p[6], *(unsigned short *)&p[8], *(unsigned short *)&p[10], 
                    *(unsigned short *)&p[12], *(unsigned short *)&p[14]);
            break;
            default:
                fprintf(stderr,"%02X %02X %02X %02X %02X %02X %02X %02X-%02X %02X %02X %02X %02X %02X %02X %02X | ",
                    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
            break;
        }
        fprintf(stderr,"%s \n", ch);

        p+=16;
        len-=16;
    }/* End of for */
    if (len)
    {
        //fprintf(stderr,"%08x: ", (p-(unsigned char *)frame));
        fprintf(stderr,"%08x: ", p);
        if (show_type == 2)
            len = ((len-1)|0x3)+1;
        else if (show_type == 1)
            len = ((len-1)|0x1)+1;
        for(i=0; i<16;)
        {
            switch(show_type)
            {
                case 2:
                    if (i<len)
                    {
                        ch[i] = (p[i]<0x20 || p[i]>=0x7f) ? '.' : p[i];
                        ch[i+1] = (p[i+1]<0x20 || p[i+1]>=0x7f) ? '.' : p[i+1];
                        ch[i+2] = (p[i+2]<0x20 || p[i+2]>=0x7f) ? '.' : p[i+2];
                        ch[i+3] = (p[i+3]<0x20 || p[i+3]>=0x7f) ? '.' : p[i+3];
                        fprintf(stderr,"%08X ",*(unsigned int *)&p[i]);
                    }
                    else
                    {
                        //ch[i]=ch[i+1]=ch[i+2]=ch[i+3]='\0';
                        ch[i]='\0';
                        fprintf(stderr,"            ");
                    }
                    i+=4;
                break;
                case 1:
                    if (i<len)
                    {
                        ch[i] = (p[i]<0x20 || p[i]>=0x7f) ? '.' : p[i];
                        ch[i+1] = (p[i+1]<0x20 || p[i+1]>=0x7f) ? '.' : p[i+1];
                        fprintf(stderr,"%04X ",*(unsigned short *)&p[i]);
                    }
                    else
                    {
                        //ch[i]=ch[i+1]='\0';
                        ch[i]='\0';
                        fprintf(stderr,"      ");
                    }
                    i+=2;
                break;
                default:
                    if (i<len)
                    {
                        ch[i] = (p[i]<0x20 || p[i]>=0x7f) ? '.' : p[i];
                        fprintf(stderr,"%02X ",p[i]);
                    }
                    else
                    {
                        ch[i]='\0';
                        fprintf(stderr,"   ");
                    }
                    i++;
                break;
            }
        }
        fprintf(stderr,"| %s \n", ch);
    }
}

//
// display log message
//
int p2p_log_msg(char *fmt, ...)
{
    struct tm TM = {0};
    time_t t = time(NULL);
    char buffer[1024] = {0};
    char format[256] = {0};

    localtime_r (&t, &TM);

    sprintf(format, "[%04u/%02u/%02u-%02u:%02u:%02u] %s", TM.tm_year + 1900, TM.tm_mon + 1, TM.tm_mday, TM.tm_hour, TM.tm_min, TM.tm_sec, fmt);

    va_list arg_list;

    va_start(arg_list, fmt);
    vsnprintf(buffer, 1024, format, arg_list);
    va_end(arg_list);

    fprintf(stderr, "%s", buffer);

    return 0;
}

//
// function: get_server_ip
//  to use system call to get default route interface IP address
//  parameter:
//      buffer start
//      buffer size
//  return
//      0: success
//      other fail
//
int get_server_ip(char *buf, int bufsize)
{
    #define _cmd1_str_  "route | grep default | awk '{print $8}' | tr -d '\n'"
    #define _cmd2_str_  "ifconfig %s | awk '/inet addr/{print substr($2,6)}' | tr -d '\n'" 
    
    char msg[256];
    int ret = SYS_OK;
    FILE *fp;
    
     // get interface
    if ((fp = popen(_cmd1_str_, "r")) == NULL) 
    {
        p2p_log_msg("%s(): Error opening pipe!\n", __FUNCTION__);
        return -1;
    }

    if (fgets(buf, bufsize, fp))
        // Do whatever you want here...
        p2p_log_msg("%s(): interface: %s\n", __FUNCTION__, buf);
        
    ret= pclose(fp);
    if (ret)  
    {
        p2p_log_msg("%s(): Command not found or exited with error status, %d\n", __FUNCTION__, ret);
        return ret;
    }
    // get ip
    snprintf(msg,255,_cmd2_str_,buf);
    //p2p_log_msg("%s!\n",msg);
    if ((fp = popen(msg, "r")) == NULL) 
    {
        p2p_log_msg("%s(): Error opening pipe!\n", __FUNCTION__);
        return -1;
    }

    if (fgets(buf, bufsize, fp))
        // Do whatever you want here...
        p2p_log_msg("%s(): IP: %s\n", __FUNCTION__, buf);
        
    ret= pclose(fp);
    if (ret)  
    {
        p2p_log_msg("%s():Command not found or exited with error status, %d\n", __FUNCTION__, ret);
        return ret;
    }
    
    return ret;
}
