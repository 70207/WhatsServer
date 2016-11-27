#ifndef _WS_PROCESS_H__
#define _WS_PROCESS_H__

#define YMZ_PROCESS_RESERVE		16

#define ws_signal_helper(n)     SIG##n
#define ws_signal_value(n)      ws_signal_helper(n)

#define ws_value_helper(n)   #n
#define ws_value(n)          ws_value_helper(n)

#define YMZ_SHUTDOWN_SIGNAL      QUIT
#define YMZ_TERMINATE_SIGNAL     TERM
#define YMZ_NOACCEPT_SIGNAL      WINCH
#define YMZ_RECONFIGURE_SIGNAL   HUP


#define YMZ_CMD_OPEN_CHANNEL   1
#define YMZ_CMD_CLOSE_CHANNEL  2
#define YMZ_CMD_QUIT           3
#define YMZ_CMD_TERMINATE      4
#define YMZ_CMD_REOPEN         5

#define YMZ_PROCESS_SINGLE				0
#define YMZ_PROCESS_MASTER				1
#define YMZ_PROCESS_WORKER				2
#define YMZ_PROCESS_DAEMON				3


typedef void (*ws_process_proc)();
typedef struct ws_process_s ws_process_t;

struct ws_process_s
{
	long long			pid;
	ws_int32_t					channel;

	ws_int32_t					status;
	const ws_char_t*			name;
	ws_process_proc	proc;
	ws_int32_t					exiting;
	ws_int32_t					exited;
};


ws_int32_t ws_worker_start_process( ws_process_proc proc );

ws_int32_t ws_spawn_process( ws_process_proc proc, const ws_char_t* name, ws_int32_t pro_id );

ws_int32_t ws_init_signals();

extern ws_process_t ws_processes[ YMZ_PROCESS_RESERVE ];
extern long long ws_pid;
extern ws_int32_t ws_child_num;
extern ws_int32_t ws_process;
extern ws_int32_t ws_process_id;
extern ws_int32_t ws_process_fd;

extern ws_int32_t ws_reap;
extern ws_int32_t ws_quit;
extern ws_int32_t ws_noaccept;
extern ws_int32_t ws_terminate;
extern ws_int32_t ws_exiting;
extern ws_int32_t ws_sigio;
extern ws_int32_t ws_sigalrm;
extern ws_int32_t ws_reconfigure;


#endif //_WS_PROCESS_H__
