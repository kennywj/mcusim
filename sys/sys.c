#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "FreeRTOS.h"
#include "task.h"
#include "sys.h"
#include "cmd.h"

const char *baudstr[MAX_BAUD_NUM]= {"9600","38400","57600","115200","921600"};
const int baudrate[MAX_BAUD_NUM]= {B9600,B38400,B57600,B115200, B921600};
unsigned char debug_flags = 0;

//
// function: exit_u2w
//
void exit_u2w()
{
	//link_control(0);	// force ppp off
}


//
//  function: cmd_sys
//      display csystem information
//  parameters
//      argc:   1
//      argv:   none
//
void cmd_sys(int argc, char* argv[])
{
    char *buf = malloc(0x4000);
    if (buf)
    {
        printf("======<Threads status>======\nName\t\tState\tPrio\tStack\tNum");
        vTaskList(buf);
        printf("%s",buf);
        printf("======<Current %u, Threads run time>======\nName\t\tSeconds\t\tPercent",xTaskGetTickCount());
        vTaskGetRunTimeStats(buf);
        printf("%s",buf);        
        free(buf);
    }
}







