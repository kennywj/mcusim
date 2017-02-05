/* states */
#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "cmd.h"

void cmd_help(int argc, char* argv[])
{
    struct cmd_tbl  *p = &commands[0];
    
    while(p && p->name!=NULL)
    {
        printf("%-10s -- %10s,\n\t%10s\n",p->name, p->desc, p->usage); 
        p++;   
    }
}

//
//  function: cmd_quit
//      exit the tunnel server daemon
//  parameters
//      argc:   1
//      argv:   none
//
void cmd_quit(int argc, char* argv[])
{
    extern int do_exit;
    do_exit = -1; // user exit
}



//=============================================================================
//  function: cmd_ver
//      diaplay device agent firmware version
//  parameters
//      argc:   0
//      argv:   NULL
//=============================================================================
void cmd_ver(int argc, char* argv[])
{
    extern const char *software_version;
    printf("%s\n",software_version);
}