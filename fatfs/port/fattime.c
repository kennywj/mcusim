#include <time.h>
#include <sys/time.h>
#include "diskio.h"		/* FatFs lower layer API */

DWORD get_fattime (void)
{
    struct timeval tv;
    time_t      secs;
    struct tm   *get_time;
    DWORD fattime = 0;

    gettimeofday(&tv, NULL);
    secs = tv.tv_sec;
    
    get_time = localtime(&secs);
    
    fattime = ((get_time->tm_year-80) << 25)
              | ((get_time->tm_mon+1)   << 21)
              | ( get_time->tm_mday     << 16)
              | ( get_time->tm_hour     << 11)
              | ( get_time->tm_min      <<  5)
              | ( get_time->tm_sec      <<  2);

    //printf("GetTime = 0x%x\n", (unsigned int) GetTime);

    return fattime;
}


/*void RTC_set_time_test(void)
{
    uint32_t year, month, day, hour,  min, sec;
    time_t set_sec;
    struct tm set_time;

    year = 2017;
    month = 06;
    day = 27;
    hour = 10;
    min = 30;
    sec = 0;
    set_time.tm_year = year-1900;
    set_time.tm_mon = month-1;
    set_time.tm_mday = day;
    set_time.tm_hour = hour;
    set_time.tm_min = min;
    set_time.tm_sec = sec;

    set_sec = mktime(&set_time);
    hal_sys_time_set(set_sec);
}*/

