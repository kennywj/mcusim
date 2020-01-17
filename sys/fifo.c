#include <stdio.h>		/* printf, scanf, NULL */
#include "fifo.h"

typedef struct _fifo_
{
	unsigned short 	put;
	unsigned short 	get;
	unsigned short 	full;
	char buf[FIFO_SIZE];
} fifo_t ;

//
// put FIFO array in fix address
// volatile static fifo_t (*fifo)[MAX_FIFO] = (fifo_t (*)[MAX_FIFO]) 0x1000; 
//

volatile static fifo_t fifo[MAX_FIFO];

int fifo_init(void)
{
	fifo_t *fp;
	
	fp = (fifo_t *)&fifo[HOST];
	fp->put = fp->get = fp->full = 0;
	fp = (fifo_t *)&fifo[DEVICE];
	fp->put = fp->get = fp->full = 0;
	return 0;
}


int fifo_put(int id, char ch)
{
	fifo_t *fp = (fifo_t *)&fifo[id];
	
	if (fp->full)
		return FIFO_FULL;
	fp->buf[fp->put]=ch;
	fp->put = (fp->put + 1)%FIFO_SIZE;
	if (fp->put == fp->get)
		fp->full = 1;
	return 0;
}

int fifo_get(int id, char *ch)
{
	fifo_t *fp = (fifo_t *)&fifo[id];
	
	if (!fp->full && fp->put == fp->get)
		return FIFO_EMPTY;
	*ch =  fp->buf[fp->get];
	fp->get = (fp->get + 1)%FIFO_SIZE;
	fp->full = 0;
	return 0;
}

int fifo_size(int id)
{
	fifo_t *fp = (fifo_t *)&fifo[id];
	if (fp->put >= fp->get)
		return fp->put - fp->get;
	else
		return FIFO_SIZE + fp->put - fp->get;
}

int fifo_space(int id)
{
	return FIFO_SIZE - fifo_size(id);
}

void fifo_dump()
{
	int i,k;
	fifo_t *fp;
	
	printf("                         ");
	for(i=0;i<FIFO_SIZE;i++)
		printf("%2d ",i);
	printf("\n");
	
	for(k=0;k<MAX_FIFO;k++)
	{
		fp = (fifo_t *)&fifo[k];
		printf("[%d] put=%2d, get=%2d, %s",k, fp->put, fp->get,fp->full?"full ":"     ");
		for(i=0;i<FIFO_SIZE;i++)
		{
			if (fp->put < fp->get)
			{
				if (i<fp->put || i>=fp->get)
				{
					printf(" %c ",fp->buf[i]);
					continue;
				}
			}
			else 
			{
				if ((i<fp->put && i>=fp->get)  || fp->full)
				{
					printf(" %c ",fp->buf[i]);
					continue;
				}
			}
			printf(" . ");
		}
		printf("\n");
	}
}