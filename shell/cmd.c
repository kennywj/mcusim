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
#include <getopt.h>
#include "cmd.h"

//
// define command support by stystem
//
struct cmd_tbl commands[]=
{
	{"help",    cmd_help, "display all commands and help",      ""},
	{"quit",    cmd_quit, "exit this program",                  ""},
	{"sys",     cmd_sys,  "display system information and status",""},
	{"json",    cmd_json, "JSON parser test program",			""},  
	//{"on",      cmd_on,   "active uart device PPP active",                 ""},
	//{"off",     cmd_off,  "deactive uart device PPP deactive",               ""},
	{"gnss",    cmd_gnss, "GNSS module control",           "[-d device name] [-b baudrate] <on|off|reset <type>|sleep <time>"},
	{"ppp",     cmd_ppp,  "PPP control commands",           "[-u username][-p password][-d device name] [-b baudrate] [-m <0|1> (0:client, 1:server)] <on|off>"},
	{"esp",		cmd_esp,	"ESP8266 wifi to uart brigde module contol command",
		"-d<device> -b<baudrate> -c<clear> -h<help> [on|off] (turn on/off esp8266)"},
//	{"os",      cmd_os,   "display OS infomation",              ""},
	{"ver",     cmd_ver,  "firmware version",                   ""},
	{"ping",    cmd_ping, "Send ECHO request to destination",   "<ip address> [-t<repeat>]"},
	{"duk",     cmd_duk,  "Javascript script interpreter",      
	    "-x\"<javascript script program string>\", -f\"<javacsript program file>\""},
	{"net",     cmd_net,  "start a http clinet connection to server",        "[-u http server url][-p serice port]<on|off>"},
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
	{"parser",  cmd_http_parser,   "http_parser test command",         ""},
	{"http",    cmd_http_client,   "HTTP clinet test command",         "-u <server url> -p <server port> [on|off]"},
	{"camera",  cmd_camera, "GP camera control",         
		"-d <device> -b <baudrate> -x<camera command string> [on|off|getpic]"},
	{"selftest", 	cmd_selftest, "mbedtls selt test",""},
	{"longopt", 	cmd_long_options, "do long option test",""},
	{"md5", 	cmd_md5, "do md5 calculate","md5 <file>"},
	{"sha1", 	cmd_sha1, "do SHA1 hash digest calculate","sha1 <file>"},
	{"fifo", 	cmd_fifo, "do FIFO read/write test","fifo -i <ID> -[r/w] <data>"},
	{"list", 	cmd_list, "demo how to used list APIs and macro"},
	{"cifar10",	cmd_cifar10,"NN Demo the CIFAR10 example"},
	{"vgg19",	cmd_vgg19,"NN Demo the VGG19 example"},
	{"pool",	cmd_pool,"NN pooling function test"},
	{NULL,        NULL}
};

void cmd_help(int argc, char* argv[])
{
    struct cmd_tbl  *p = &commands[0];
    int count;
	
	if (argc >=2)
	{
		while(p && strcmp(argv[1],p->name)!=0)
			p++;
		if (p)
			printf("%-10s -- %10s,\n\t    %s\n",p->name, p->desc, p->usage); 
	}
	else
	{	
		while(p && p->name!=NULL)
		{
			printf("%-10s",p->name); 
			p++;   
			count ++;
			if ((count % 5)==0)
				printf("\n");
		}
		printf("\n\nInput \"quit\" to exit program\n");
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



/* Flag set by ．--verbose・. */
static int verbose_flag;

void
cmd_long_options (int argc, char *argv[])
{
  int c;

  while (1)
  {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose", no_argument,       &verbose_flag, 1},
          {"brief",   no_argument,       &verbose_flag, 0},
          /* These options don・t set a flag.
             We distinguish them by their indices. */
          {"add",     no_argument,       0, 'a'},
          {"append",  no_argument,       0, 'b'},
          {"delete",  required_argument, 0, 'd'},
          {"create",  required_argument, 0, 'c'},
          {"file",    required_argument, 0, 'f'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "abc:d:f:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'a':
          puts ("option -a\n");
          break;

        case 'b':
          puts ("option -b\n");
          break;

        case 'c':
          printf ("option -c with value `%s'\n", optarg);
          break;

        case 'd':
          printf ("option -d with value `%s'\n", optarg);
          break;

        case 'f':
          printf ("option -f with value `%s'\n", optarg);
          break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    }

  /* Instead of reporting ．--verbose・
     and ．--brief・ as they are encountered,
     we report the final status resulting from them. */
  if (verbose_flag)
    puts ("verbose flag is set");

  /* Print any remaining command line arguments (not options). */
  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      putchar ('\n');
    }
}

