#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <getopt.h>
#include "sys.h"
#include "cmd.h"
#include "vphy.h"
/**
  * @brief  BLE LL control commands
  * @param  number of arguments
  *	@param  arguments array
  * @retval None
  */
void cmd_ble(int argc, char* argv[])
{
	int c;
	char buf[256];
	
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

		c = getopt_long (argc, argv, "hedp:x:",
                       ble_options, &option_index);

      /* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 'e':
				vphy_init(NULL);
			break;
			case 'd':
				vphy_exit();
			break;
			case 'p':
				vphy_ctrl(1, strlen(optarg), (char *)optarg);
			break;
			case 'x': // transmit PDU
				vphy_output((char *)optarg, strlen(optarg));
			break;
			case 'h':
				printf("BLE LL commands\n"
				"Usage:-e enable VPHY, -d disable VPHY, -p <device name> physical device name\n");
			break;
			default:
			break;
		}
	}	// end while
	// show BLE LL status
	vphy_ctrl(0, 255, buf);
	printf("%s", buf);
	return;
}