//
// struct fot TX FIFO
//
struct _queue_
{
	//SemaphoreHandle_t	sem;
	unsigned int 		put;
	unsigned int 		get;
	unsigned int 		full;
	unsigned int 		size;	// buf size
	char *buf;
};

extern int queue_init(struct _queue_ *q, int size);
extern void queue_exit(struct _queue_ *q);
extern int queue_put(struct _queue_ *q, char *data, int size);
extern int queue_get(struct _queue_ *q,char *buf, int bufsize);
extern int queue_peek(struct _queue_ *q, char *buf, int bufsize);
extern int queue_move(struct _queue_ *q, int bufsize);
extern int queue_data_size(struct _queue_ *q);
extern int queue_space(struct _queue_ *q);
extern int queue_reset(struct _queue_ *q);
