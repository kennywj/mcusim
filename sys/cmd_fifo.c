#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <getopt.h>
#include "fifo.h"
#include "cmd.h"

//
//  function: cmd_fifo
//      do FIFO read/write test
//		-i <0|1>
//		-r <number>
//		-w <data string>
//  parameters
//      argc: 	1
//      argv:   none
//
void cmd_fifo(int argc, char* argv[])
{
	int c,i,n;
	char ch, *cp;
	static int id=HOST;
	// start a thread to do ssl client 
	while((c=getopt(argc, argv, "i:r:w:h?")) != -1)
    {
        switch(c)
        {
		case 'i':
			id = atoi(optarg)%MAX_FIFO;
			printf("set FIFO ID %d\n",id);
			break;
	    case 'r':
	    	n = atoi(optarg);
	    	for (i=0;i<n;i++)
	    	{
	    		if (fifo_get(id, &ch)<0)
					printf("FIFO %d empty\n",id);
				else
					printf("%c",ch);
			}
			printf("\n");
			break;
	    case 'w':
	    	cp = optarg;
	    	while(*cp)
	    	{
	    		if (fifo_put(id, *cp)<0)
	    		{
	    			printf("FIFO %d full\n", id);
	    			break;
	    		}
	    		cp++;
	    	}
			break;
        default:
            printf("wrong command!\n");
		case 'h':
		case '?':
			printf("usage: %s\n",curr_cmd->usage);
			return;
        }
    }   // end while
    // display FIFO contain
    fifo_dump();
}
