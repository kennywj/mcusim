#include <stdio.h>
#include <string.h>
#include "allystar.h"


extern int send_out(char *buf, int len);


unsigned short int allystar_checksum(unsigned char *data, int count)
{
    unsigned char sum1 = 0;
    unsigned char sum2 = 0;
    int index;

    for( index = 0; index < count; ++index )
    {
        sum1 = sum1 + data[index];
        sum2 = sum2 + sum1;
    }
    return ((unsigned short int)sum2 << 8) | sum1;
}


int allystar_reset(unsigned char type)
{
	unsigned char msg[9]={0xf1, 0xd9, 0x06, 0x40, 0, 0, 0, 0};
	unsigned char *start=&msg[2], *cp=&msg[4];
	unsigned short checksum, len = 1;
	int total;
	
	*cp++ = len & 0xFF;			/*length low*/
	*cp++ = (len>>8) & 0xFF;	/*length high*/
	*cp++ = type;
	checksum = allystar_checksum( start, (int)cp-(int)start);
	*cp++ = checksum & 0xFF;
	*cp++ = (checksum >> 8) & 0xFF;
	total =  (int)cp-(int)msg;
	dump_frame(msg, total, "Send reset, type=%d\n",type);
	return send_out((char *)msg, total);
}

int allystar_sleep(unsigned int ms, unsigned char action)
{
	unsigned char msg[13]={0xf1, 0xd9, 0x06, 0x41, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char *start=&msg[2], *cp=&msg[4];
	unsigned short checksum, len = 5;
	int total;
	
	*cp++ = len & 0xFF;	/*length low*/
	*cp++ = (len>>8) & 0xFF;	/*length high*/
	*cp++ = ms & 0xFF;
	*cp++ = (ms>>8) & 0xFF;
	*cp++ = (ms>>16) & 0xFF;
	*cp++ = (ms>>24) & 0xFF;
	*cp++ = action;
	checksum = allystar_checksum( start, (int)cp-(int)start);
	*cp++ = checksum & 0xFF;
	*cp++ = (checksum >> 8) & 0xFF;
	total =  (int)cp-(int)msg;
	dump_frame(msg, total, "Send sleep, time=%ums, action = %d\n",ms, action);
	return send_out((char *)msg, total);
}