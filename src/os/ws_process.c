#include <ws_std.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <ws_cycle.h>
#include <ws_process.h>
#include <ws_process_pipe.h>


ws_int32_t ws_child_num = 0;
long long ws_pid = 0;
ws_int32_t ws_process = 0;
ws_int32_t ws_process_id = 0;
ws_int32_t ws_process_fd = 0;
ws_process_pipe_t ws_process_pipe;




void ws_signal_handler(ws_int32_t signo);
static void ws_process_get_status(void);

ws_process_t ws_processes[ YMZ_PROCESS_RESERVE ];

ws_int32_t ws_worker_start_process( ws_process_proc proc )
{
	ws_int32_t worker_process_num;
	ws_int32_t i;

	memset( ws_processes, 0, YMZ_PROCESS_RESERVE * sizeof( ws_process_t ) );

	worker_process_num = ws_cycle->worker_process_num;
	ws_child_num = worker_process_num;
	ws_process_id = 0;

	for( i = 0; i < worker_process_num; ++i ){
		ws_spawn_process( proc, "worker name", i );
	}

	return 0;
}

ws_int32_t ws_spawn_process( ws_process_proc proc, const ws_char_t* name, ws_int32_t pro_id )
{
#ifdef __linux__
	pid_t							pid;
	ws_int32_t								pfd;
	ws_int32_t								pp[ 2 ];

	pipe( pp );

	if( ws_processes[ pro_id ].channel )
	{
		close( ws_processes[ pro_id ].channel );
	}

	ws_processes[ pro_id ].channel = pp[ 1 ];
	pfd = pp[ 0 ];

	pid = fork();
		
	switch( pid ){
	case -1:
		return YMZ_SYSTEM_ERROR;
	case 0:

		close( pp[ 1 ] );
		ws_process_id = pro_id + 1;
		ws_process_fd = pfd;
		ws_process_pipe.slot = pfd;
		ws_pid = getpid();
		proc();
		break;

	default:
		break;
	}

	close( pp[ 0 ] );
	ws_processes[ pro_id ].proc = proc;
	ws_processes[ pro_id ].pid = pid;
	ws_processes[ pro_id ].exited = 0;
	ws_processes[ pro_id ].exiting = 0;
	ws_processes[ pro_id ].name = name;

#endif
	return YMZ_OK;
}



typedef struct {
    ws_int32_t     signo;
    char   *signame;
    char   *name;
    void  (*handler)(ws_int32_t signo);
} ws_signal_t;

#ifdef __linux__
ws_signal_t  signals[] = {
    { ws_signal_value(YMZ_RECONFIGURE_SIGNAL),
      "SIG" ws_value(YMZ_RECONFIGURE_SIGNAL),
      "reload",
      ws_signal_handler },

  
    { ws_signal_value(YMZ_NOACCEPT_SIGNAL),
      "SIG" ws_value(YMZ_NOACCEPT_SIGNAL),
      "",
      ws_signal_handler },

    { ws_signal_value(YMZ_TERMINATE_SIGNAL),
      "SIG" ws_value(YMZ_TERMINATE_SIGNAL),
      "stop",
      ws_signal_handler },

    { ws_signal_value(YMZ_SHUTDOWN_SIGNAL),
      "SIG" ws_value(YMZ_SHUTDOWN_SIGNAL),
      "quit",
      ws_signal_handler },


    { SIGALRM, "SIGALRM", "", ws_signal_handler },

    { SIGINT, "SIGINT", "", ws_signal_handler },

    { SIGIO, "SIGIO", "", ws_signal_handler },

    { SIGCHLD, "SIGCHLD", "", ws_signal_handler },

    { SIGSYS, "SIGSYS, SIG_IGN", "", SIG_IGN },

    { SIGPIPE, "SIGPIPE, SIG_IGN", "", SIG_IGN },

    { 0, NULL, "", NULL }
};

#endif


ws_int32_t ws_init_signals( )
{
#ifdef __linux__
    ws_signal_t      *sig;
    struct sigaction   sa;

    for (sig = signals; sig->signo != 0; sig++) {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = sig->handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(sig->signo, &sa, NULL) == -1) {
            return YMZ_ERROR;
        }
    }

#endif
    return YMZ_OK;
}



void
ws_signal_handler(ws_int32_t signo)
{
#ifdef __linux__
    char						*action;
    ws_int32_t							 ignore;
    ws_int32_t							 err;
    ws_signal_t			    *sig;

    ignore = 0;

    err = errno;

    for (sig = signals; sig->signo != 0; sig++) {
        if (sig->signo == signo) {
            break;
        }
    }

   // ngx_time_sigsafe_update();

    action = "";

    switch (ws_process) {

    case YMZ_PROCESS_MASTER:
    case YMZ_PROCESS_SINGLE:
        switch (signo) {

        case ws_signal_value(YMZ_SHUTDOWN_SIGNAL):
            ws_quit = 1;
            action = ", shutting down";
            break;

        case ws_signal_value(YMZ_TERMINATE_SIGNAL):
        case SIGINT:
            ws_terminate = 1;
            action = ", exiting";
            break;

        case ws_signal_value(YMZ_NOACCEPT_SIGNAL):
            
             ws_noaccept = 1;
             action = ", stop accepting connections";
            
            break;

        case ws_signal_value(YMZ_RECONFIGURE_SIGNAL):
            ws_reconfigure = 1;
            action = ", reconfiguring";
            break;

 
        case SIGALRM:
            ws_sigalrm = 1;
            break;

        case SIGIO:
            ws_sigio = 1;
            break;

        case SIGCHLD:
            ws_reap = 1;
            break;
        }

        break;

    case YMZ_PROCESS_WORKER:
        switch (signo) {

    
        case ws_signal_value(YMZ_SHUTDOWN_SIGNAL):
            ws_quit = 1;
            action = ", shutting down";
            break;

        case ws_signal_value(YMZ_TERMINATE_SIGNAL):
        case SIGINT:
            ws_terminate = 1;
            action = ", exiting";
            break;

        case ws_signal_value(YMZ_RECONFIGURE_SIGNAL):
        case SIGIO:
            action = ", ignoring";
            break;
        }

        break;
    }

	if( SIGCHLD == signo ){
		ws_process_get_status();
	}

#endif
}


static void
ws_process_get_status(void)
{
#ifdef __linux__
    ws_int32_t              status;
    char            *process;
    pid_t			 pid;
    ws_int32_t	             err;
    ws_int32_t		         i;
    ws_uint32_t     one;

    one = 0;

    for ( ;; ) {
        pid = waitpid(-1, &status, WNOHANG);

        if (pid == 0) {
            return;
        }

        if (pid == -1) {
            err = errno;

            if (err == EINTR) {
                continue;
            }

            if (err == ECHILD && one) {
                return;
            }

            /*
             * Solaris always calls the signal handler for each exited process
             * despite waitpid() may be already called for this process.
             *
             * When several processes exit at the same time FreeBSD may
             * erroneously call the signal handler for exited process
             * despite waitpid() may be already called for this process.
             */

            if (err == ECHILD) {
                return;
            }

            return;
        }


        one = 1;
        process = "unknown process";

        for (i = 0; i < ws_child_num; i++) {
            if (ws_processes[i].pid == pid) {
                ws_processes[i].status = status;
                ws_processes[i].exited = 1;
                process = ws_processes[i].name;
                break;
            }
        }

    }

#endif
}
