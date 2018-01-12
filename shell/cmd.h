//
// include file for buildin command
//
#ifndef __CMD_H__
#define __CMD_H__

//#define MAX_ARGS 12

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
//extern void cmd_cfg(int argc, char* argv[]);
extern void cmd_ver(int argc, char* argv[]);
//extern void cmd_on(int argc, char* argv[]);
//extern void cmd_off(int argc, char* argv[]);
//extern void cmd_xmt(int argc, char* argv[]);
//extern void cmd_os(int argc, char* argv[]);
extern void cmd_ping(int argc, char* argv[]);
extern void cmd_duk(int argc, char* argv[]);
extern void cmd_net(int argc, char* argv[]);
extern void cmd_xmodem(int argc, char* argv[]);
extern void cmd_touch(int argc, char* argv[]);
extern void cmd_dir(int argc, char* argv[]);
extern void cmd_del(int argc, char* argv[]);
extern void cmd_mkdir(int argc, char* argv[]);
extern void cmd_cd(int argc, char* argv[]);
extern void cmd_pwd(int argc, char* argv[]);
extern void cmd_rename(int argc, char* argv[]);
extern void cmd_stat(int argc, char* argv[]);
extern void cmd_cp(int argc, char* argv[]);
extern void cmd_cat(int argc, char* argv[]);
extern void cmd_echo(int argc, char* argv[]);
extern void cmd_http(int argc, char* argv[]);
extern void cmd_camera(int argc, char* argv[]);
extern void cmd_ppp(int argc, char* argv[]);
extern void cmd_esp(int argc, char* argv[]);
extern void cmd_selftest(int argc, char* argv[]);

extern void dump_frame(char *frame, int len, const char * fmt, ...);
#endif
