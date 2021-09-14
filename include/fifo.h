
#ifndef _FIFO_H_
#define _FIFO_H_
#include "error.h"

#define FIFO_SIZE	16
//#define FIFO_SIZE	512

#define HOST 		0
#define DEVICE 		1
#define MAX_FIFO	2

#define FIFO_FULL 	-1
#define FIFO_EMPTY	-2


extern int fifo_init(void);
extern int fifo_put(int id, char ch);
extern int fifo_get(int id, char *ch);
extern int fifo_size(int id);
extern int fifo_space(int id);
extern void fifo_dump(void);
#endif