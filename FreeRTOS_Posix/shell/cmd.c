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
#include "duktape.h"

//
// define command support by stystem
//
struct cmd_tbl commands[]=
{
	{"help",    cmd_help, "display all commands and help",      ""},
	{"quit",    cmd_quit, "exit this program",                  ""},
	{"stat",    cmd_stat, "display uart2wifi status",           ""},
	{"cfg",     cmd_cfg,  "program uart2wifi configuration",  "-p <device> -b <baud> -t<type>(0:PPP client, 1:server)"},
	{"on",      cmd_on,   "active uart device PPP active",                 ""},
	{"off",     cmd_off,  "deactive uart device PPP deactive",               ""},
	//{"xmt",     cmd_xmt,  "sent out command message",           "\"message string\""},
	{"os",      cmd_os,   "display OS infomation",              ""},
	{"ver",     cmd_ver,  "firmware version",                   ""},
	{"ping",    cmd_ping, "Send ECHO request to destination",   "<ip address> [-t<repeat>]"},
	{"duk",     cmd_duk,  "Javascript script interpreter",      "-x\"<javascript script program string>\", -f\"<javacsript program file>\""},
	{"net",     cmd_net,  "display network information",        ""},
	{NULL,        NULL}
};

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
void cmd_os(int argc, char* argv[])
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
}

//
//  function cmd_duk
//      javascript command line interpereter
//  parameters
//      argc: 2
//      argv: duk -x "<javascript script program string>"
//            duk -f "<javacsript program file>"
//
static void push_file_as_string(duk_context *ctx, const char *filename);
static duk_ret_t native_print(duk_context *ctx);
static duk_ret_t native_adder(duk_context *ctx);

void cmd_duk(int argc, char* argv[])
{
    duk_context *ctx;
    int c;
    if (argc<3)
    {
        printf("argument not enough!\n usgae: %s\n",curr_cmd->usage);
        return;
    }    
    
    ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        return;
    }
    
    duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");
	duk_push_c_function(ctx, native_adder, DUK_VARARGS);
	duk_put_global_string(ctx, "adder");
	
    while((c=getopt(argc, argv, "x:f:")) != -1)
    {
        switch(c)
        {
            case 'x':
                duk_push_lstring(ctx, (const char *) argv[2], (duk_size_t) strlen(argv[2]));
            break;
            case 'f':
                push_file_as_string(ctx, argv[2]);
            break;
            default:
                printf("wrong argument!\n usgae: %s\n",curr_cmd->usage);
                duk_push_undefined(ctx);
                break;
        }
        if (duk_peval(ctx) != 0) {
            printf("Error: %s\n", duk_safe_to_string(ctx, -1));
            break;  // exit while loop
        }
        duk_pop(ctx);  /* ignore result */
    } 
end_cmd_duk:       
    duk_destroy_heap(ctx);
}

static duk_ret_t native_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t native_adder(duk_context *ctx) {
	int i;
	int n = duk_get_top(ctx);  /* #args */
	double res = 0.0;

	for (i = 0; i < n; i++) {
		res += duk_to_number(ctx, i);
	}

	duk_push_number(ctx, res);
	return 1;  /* one return value */
}

// For brevity assumes a maximum file length of 16kB.
static void push_file_as_string(duk_context *ctx, const char *filename) {
    FILE *f;
    size_t len;
    char *buf;
    #define BUF_SIZE 16384
    
    buf = malloc(BUF_SIZE);
    if (!buf)
    {
        printf("no memory");
        duk_push_undefined(ctx);
        return;
    }    
    f = fopen(filename, "rb");
    if (f) {
        len = fread((void *) buf, 1, BUF_SIZE, f);
        fclose(f);
        duk_push_lstring(ctx, (const char *) buf, (duk_size_t) len);
    } else {
        duk_push_undefined(ctx);
    }
    free(buf);
}

