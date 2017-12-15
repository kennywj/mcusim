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

//
// define command support by stystem
//
struct cmd_tbl commands[]=
{
	{"help",    cmd_help, "display all commands and help",      ""},
	{"quit",    cmd_quit, "exit this program",                  ""},
	{"sys",     cmd_sys, "display system information and status",           ""},
	//{"cfg",     cmd_cfg,  "program uart2wifi configuration",  
	//    "-d <device> -b <baud> -m<type>(0:PPP client, 1:server),"
	//    "\t    -u <username> -p <password> -c (clear)"},
	//{"on",      cmd_on,   "active uart device PPP active",                 ""},
	//{"off",     cmd_off,  "deactive uart device PPP deactive",               ""},
	{"ppp",     cmd_ppp,  "PPP control commands",           ""},
//	{"os",      cmd_os,   "display OS infomation",              ""},
	{"ver",     cmd_ver,  "firmware version",                   ""},
	{"ping",    cmd_ping, "Send ECHO request to destination",   "<ip address> [-t<repeat>]"},
	{"duk",     cmd_duk,  "Javascript script interpreter",      
	    "-x\"<javascript script program string>\", -f\"<javacsript program file>\""},
	{"net",     cmd_net,  "display network information",        ""},
	{"echo",     cmd_echo,  "tcp echo client/server",        
		"-g (start) [-t<time>] <message> -p <port> -s<server url> -m<message>"},
	{"xmodem",  cmd_xmodem,  "xmodem client, to xmr/rcv data via UART/xmodem protocol",
	    "-r(read data from console) | -w (write data to console)\n"
	    "\t    -a <start address for xmt> -l <data/buffer size>\n"},
	{"touch",   cmd_touch,  "create a new file",        "<filename> [<data>] (option)"},
	{"dir",     cmd_dir,    "list files/folders",        ""},
	{"ls",      cmd_dir,    "list files/folders",        ""},
	{"del",     cmd_del,    "delete files/folders",        "[-r (recursive)] <file/folder name>"},
	{"rm",      cmd_del,    "delete files/folders",        "[-r (recursive)] <file/folder name>"},
	{"mkdir",   cmd_mkdir,  "create a new folder",        "<new folder name>"},
	{"cd",      cmd_cd,     "change currently work folder",        "<folder>"},
	{"pwd",     cmd_pwd,    "display currently work folder",        ""},
	{"rename",  cmd_rename, "change file/floder's name",        "<old name> <new name>"},
	{"stat",    cmd_stat,   "show file/floder's status",        "<file/folder name>"},
	{"cp",      cmd_cp,     "copy file",         "<original file name> <new file name>"},
	{"cat",     cmd_cat,    "concate 2 files, only one argument, show the file content",
	    "[-o overwrite] <dest file> [< surce file>]"},
	{"http",    cmd_http,   "http_parser test command",         ""},
	{"camera",  cmd_camera, "GP camera control",         
		"-d <device> -b <baudrate> -x<camera command string> [on|off|getpic]"},
	{NULL,        NULL}
};

void cmd_help(int argc, char* argv[])
{
    struct cmd_tbl  *p = &commands[0];
    
    while(p && p->name!=NULL)
    {
        printf("%-10s -- %10s,\n\t    %s\n",p->name, p->desc, p->usage); 
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
    extern void exit_u2w(void);

    do_exit = 1; // user exit
    exit_u2w();
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

//
//  function: cmd_os
//      display currently OS status
//  parameters
//      argc:   1
//      argv:   none
//
/*void cmd_os(int argc, char* argv[])
{
    char *buf = malloc(0x4000);
    if (buf)
    {
        printf("======<Threads status>======\nName\t\tState\tPrio\tStack\tNum");
        vTaskList(buf);
        printf("%s",buf);
        printf("======<Current %u, Threads run time>======\nName\t\tSeconds\t\tPercent",xTaskGetTickCount());
        vTaskGetRunTimeStats(buf);
        printf("%s",buf);        
        free(buf);
    }
}*/
