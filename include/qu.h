//
// struct fot TX FIFO
//
#ifndef _QU_H_
#define _QU_H_
struct _queue_
{
	//xSemaphoreHandle	sem;
	unsigned int 		put;
	unsigned int 		get;
	unsigned int 		full;
	unsigned int 		size;	// buf size
	char *buf;
};

#define QUEUE_OK			0
#define QUEUE_NOMEM_ERR		-1
#define QUEUE_COLLI_ERR		-2
#define QUEUE_FULL_ERR		-3
#define QUEUE_NOTAVAIL_ERR	-4

extern int queue_init(struct _queue_ *q, int size);
extern void queue_exit(struct _queue_ *q);
extern int queue_put(struct _queue_ *q, char *data, int size);
extern int queue_get(struct _queue_ *q,char *buf, int bufsize);
extern int queue_peek(struct _queue_ *q, char *buf, int bufsize);
extern int queue_move(struct _queue_ *q, int bufsize);
extern int queue_data_size(struct _queue_ *q);
extern int queue_space(struct _queue_ *q);
extern int queue_reset(struct _queue_ *q);

#endif
// end of define _QU_H_
