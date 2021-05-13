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
	{"ver",     cmd_ver,  "firmware version",                   ""},
	{"vphy",    cmd_vphy,  "BLE LL control commands",            ""},
	{NULL,        NULL}
};

/**
  * @brief  display help meesage of commands
  * @param  number of arguments
  *	@param  arguments array
  * @retval None
  */
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

/**
  * @brief  quit simulator system
  * @param  number of arguments
  *	@param  arguments array
  * @retval None
  */
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

