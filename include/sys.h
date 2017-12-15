//
// system configuration define
//
#ifndef __SYS_H__
#define __SYS_H__
#include "qu.h"
#define MAX_PKT_SIZE    1520
#define DEVICE_NAME_LEN 64
#define MAX_BAUD_NUM    5

extern const char *baudstr[MAX_BAUD_NUM];
extern const int baudrate[MAX_BAUD_NUM];

extern int link_control(int action);
#endif
