#include <stdio.h>
#include <string.h>

extern int send_out(char *buf, int len);

typedef struct _conf_pkt_type_
{
    unsigned short int msg_id;
    unsigned short int len;
    unsigned char *payload;
} ALLYSTART_MSG;

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

static int allystar_send_out(ALLYSTART_MSG *pkt)
{
	unsigned char msg[256];
    unsigned short int checksum;
	
	msg[0] = 0xF1;
    msg[1] = 0xD9;
    msg[2] = pkt->msg_id  & 0xFF;
    msg[3] = (pkt->msg_id >> 8)  & 0xFF;
    msg[4] = pkt->len  & 0xFF;
    msg[5] = (pkt->len >> 8)  & 0xFF;
    if (pkt->len)
    {
       memcpy(&msg[6], &pkt->payload[0], pkt->len);
    }
    checksum = allystar_checksum( &msg[2], 4+ pkt->len);

    msg[5+pkt->len +1 ] = checksum & 0xFF;
    msg[5+pkt->len +2 ] = (checksum >> 8) & 0xFF;
	
    return send_out((char *)msg, 8+pkt->len);
}
