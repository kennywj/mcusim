//
// header file of common uart2wifi definition
//
#ifndef _UART2WIFI_H_
#define _UART2WIFI_H_
#include "qu.h"

#define SYS_OK               0
#define SYS_NOMEM           -1
#define SYS_QUEUE_FULL      -2
#define SYS_NOT_AVAILABLE   -3

extern int _inbyte(int msec);
extern void _outbyte(unsigned char c);
extern int p2p_log_msg(char *fmt, ...);
extern int fd_set_blocking(int fd, int blocking);

#endif
// end of _UART2WIFI_H_