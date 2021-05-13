//
// include file for buildin command
//
#ifndef __CMD_H__
#define __CMD_H__

struct cmd_tbl
{
	const char *name;
	void (*func)(int argc, char *argv[]);
	const char *desc;
	const char *usage;
};
extern struct cmd_tbl commands[];
extern struct cmd_tbl *curr_cmd;
extern void cmd_quit(int argc, char* argv[]);
extern void cmd_help(int argc, char* argv[]);
extern void cmd_sys(int argc, char* argv[]);
extern void cmd_ver(int argc, char* argv[]);
extern void cmd_ble(int argc, char* argv[]);

extern void dump_frame(char *frame, int len, const char * fmt, ...);
#endif
