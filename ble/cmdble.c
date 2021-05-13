#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <getopt.h>
#include "sys.h"
#include "cmd.h"
#include "vphy.h"

static const char *Usage = 	"enable VPHY: $vphy on\n"
							"disable VPHY: $vphy off\n"
							"program physical device: $vphy -d <device name>\n"
							"transmit data: $vphy -x <message string>\n"
							"display help: $vphy -h\n"
							"display VPH status: $vphy\n";

/**
  * @brief  BLE LL control commands
  * @param  number of arguments
  *	@param  arguments array
  * @retval None
  */
void cmd_vphy(int argc, char* argv[])
{
	int c;
	char buf[256];
	static void (*recv_handler)(char *, int len) = NULL;
	while(1)
	{
		static struct option ble_options[] =
        {
			{"help",	no_argument,       0, 'h'},
			{"enable",	no_argument,       0, 'e'},
			{"disable",	no_argument,       0, 'd'},
			{"vphy",  	required_argument, 0, 'p'},
			{"xmt",  	required_argument, 0, 'x'},
			{0, 0, 0, 0}
        };
		
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "hd:x:",
                       ble_options, &option_index);

      /* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 'd':
				vphy_ctrl(1, strlen(optarg), (char *)optarg);
			break;
			case 'x': // transmit PDU
				vphy_output((char *)optarg, strlen(optarg));
			break;
			
			default:
				printf("unknown option \'%c\'\n",c);
			case 'h':
				printf("%s",Usage);
			break;
		}
	}	// end while
	
	for(;optind<argc;optind++)
	{
		if (strcmp(argv[optind],"on")==0)
			vphy_init(recv_handler);
		else if (strcmp(argv[optind],"off")==0)
			vphy_exit();
	}
	// show BLE LL status
	vphy_ctrl(0, 255, buf);
	printf("%s", buf);
	return;
}