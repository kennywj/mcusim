//
// system configuration define
//
#ifndef __SYS_H__
#define __SYS_H__
#include "qu.h"

#define SYS_OK               0
#define SYS_NOMEM           -1
#define SYS_QUEUE_FULL      -2
#define SYS_NOT_AVAILABLE   -3

#define MAX_PKT_SIZE    1520
#define DEVICE_NAME_LEN 64
#define MAX_BAUD_NUM    5

extern const char *baudstr[MAX_BAUD_NUM];
extern const int baudrate[MAX_BAUD_NUM];

extern int link_control(int action);
extern int p2p_log_msg(char *fmt, ...);
extern int fd_set_blocking(int fd, int blocking);
extern void dump_frame(char *start, int len, const char * fmt, ...);
#endif
