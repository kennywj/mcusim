
#ifndef _ALLYSTAR_H_
#define _ALLYSTAR_H_

typedef struct allystar_msg_t
{
	unsigned char hd[2];
	unsigned char id[2];
	unsigned short len;
	unsigned char  payload[0];
} ALLYSTART_MSG;

#endif