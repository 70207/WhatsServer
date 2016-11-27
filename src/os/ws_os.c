#include <ws_std.h>


#include <signal.h>
#include <fcntl.h>

#include <ws_types.h>
#include <ws_mem.h>
#include <ws_time.h>
#include <ws_cycle.h>
#include <ws_event.h>
#include <ws_process.h>
#include <ws_process_pipe.h>
#include <ws_thread.h>
#include <ws_os.h>
#include <ws_log.h>




ws_int32_t ws_reap = 0;
ws_int32_t ws_quit = 0;
ws_int32_t ws_noaccept = 0;
ws_int32_t ws_terminate = 0;
ws_int32_t ws_exiting = 0;
ws_int32_t ws_sigalrm = 0;
ws_int32_t ws_sigio = 0;
ws_int32_t ws_reconfigure = 0;

void ws_os_child_process();
void ws_os_reap_child_process();
void ws_signal_worker_processes( ws_int32_t signo  );
ws_int32_t ws_os_child_process_init();
static void ws_worker_process_exit( );
static ws_int32_t ws_channel_handler( ws_event_t *ev );
ws_int32_t ws_os_single_process_init();
void ws_single_main_thread(void* data);

ws_int32_t ws_started_check()
{
	ws_int32_t					fd;
	ws_int32_t					rc;
	
#ifdef __linux__
	fd = 0;
	if (!ws_cycle->working_lock_path){
		printf("process lock must be fill\n");
		return YMZ_ERROR;
	}

    if (ws_cycle->working_lock_path[0] == 0) {
        ws_cycle->working_lock_path = "/tmp/ws.lock";
    }

	fd = open(ws_cycle->working_lock_path, O_CREAT | O_WRONLY);
	if( fd <= 0 ){
		return YMZ_ERROR;
	}

	rc = flock( fd, LOCK_EX | LOCK_NB );
	if( rc < 0 ){
		return YMZ_ERROR;
	}

#endif
	return YMZ_OK;

}

void ws_os_master_process()
{
#ifdef __linux__
	sigset_t						set;

	sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGIO);
    sigaddset(&set, SIGINT);
	sigaddset(&set, SIGPIPE);
 

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        printf("sigprocmask() failed");
    }

    sigemptyset(&set);

	ws_worker_start_process( ws_os_child_process );

	ws_process = YMZ_PROCESS_MASTER;
	for( ;; ){
		  sigsuspend(&set);
		  ws_time_update();

		  if( ws_reap ){
			  ws_reap = 0;
			  ws_os_reap_child_process();
		  }

		  if( ws_quit ){
			  ws_signal_worker_processes( ws_signal_value( YMZ_SHUTDOWN_SIGNAL ) );
			  ws_listen_close();
			  
		  }

		  if( ws_noaccept ){
			  ws_noaccept = 0;
			  ws_signal_worker_processes( ws_signal_value( YMZ_SHUTDOWN_SIGNAL ) );
		  }

	}

#endif

}

static ws_int32_t
ws_channel_handler( ws_event_t *ev )
{
    ws_int32_t								 n;
	ws_process_pipe_t				 pp;
    ws_conn_t						*c;


    c = ev->data;

    for ( ;; ) {

        n = ws_process_pipe_read( ev->fd, &pp, sizeof(ws_process_pipe_t) );

        if ( n == YMZ_ERROR ) {
            ws_log_print_debug("channel pipe error, so close port:%d", c->port);
			ws_event_del_conn( c );
            ws_conn_close( c );
            return YMZ_ERROR;
        }

        if ( n == YMZ_AGAIN ) {
            return YMZ_AGAIN;
        }

     
        switch (pp.cmd) {

        case YMZ_CMD_QUIT:
            ws_quit = 1;
            break;

        case YMZ_CMD_TERMINATE:
            ws_terminate = 1;
            break;
		}
    }

	return YMZ_OK;
}


void ws_signal_worker_processes( ws_int32_t signo  )
{
#ifdef __linux__
	ws_process_pipe_t			pp;
	ws_int32_t							i;

	switch( signo ){
	case ws_signal_value(YMZ_SHUTDOWN_SIGNAL):
		pp.cmd = YMZ_CMD_QUIT;
		break;

	case ws_signal_value(YMZ_TERMINATE_SIGNAL):
		pp.cmd = YMZ_CMD_TERMINATE;
		break;

	default:
		pp.cmd = 0;
	}

	for( i = 0; i < ws_child_num; ++i ){
		if( ws_processes[i].exiting ){
			continue;
		}

		if( pp.cmd ){
			ws_process_pipe_write( ws_processes[ i ].channel, &pp, sizeof( ws_process_pipe_t ) );
			ws_processes[ i ].exiting = 1;
		}

		if( kill( ws_processes[ i ].pid, signo ) == -1 ){
			ws_processes[ i ].exited = 1;
			ws_processes[ i ].exiting = 0;
			ws_reap = 1;
		}

	}

#endif
}

void ws_os_reap_child_process()
{
	ws_int32_t					i;

	for( i = 0; i < ws_child_num; ++i ){
		if( ws_processes[ i ].pid == -1 ){
			continue;
		}


		if( ws_processes[ i ].exited ){
			ws_spawn_process( ws_processes[ i ].proc, ws_processes[ i ].name, i );
		}		

	}
}



void ws_os_single_process()
{	
  ws_thread_regist(ws_single_main_thread, 0, &ws_exiting);
		
}

void ws_single_main_thread(void* data) {
    ws_int32_t						i;
    ws_conn_t*				c;
    ws_event_t*			rev;

    if (ws_exiting) {
        c = ws_cycle->conns;
        for (i = 0; i < ws_cycle->conn_num; ++i) {
            if (c[i].s > 0) {
                c[i].close = 1;
                rev = c[i].rev;
                if (rev) {
                    rev->handle(rev);
                }
            }
        }

        ws_worker_process_exit();

    }



    if (ws_terminate) {
        ws_worker_process_exit();
    }

    if (ws_quit) {
        ws_listen_close();
        if (!ws_exiting) {
            ws_exiting = 1;
        }
    }

    ws_cycling();

}

void ws_os_child_process()
{
		ws_int32_t						i;
		ws_conn_t*				c;
		ws_event_t*				rev;

		ws_process = YMZ_PROCESS_WORKER;
		if( ws_os_child_process_init() ){
			ws_exiting = 1;
		}

		for(;;){
			if( ws_exiting ){
				c = ws_cycle->conns;
				for( i = 0; i < ws_cycle->conn_num; ++i ){
					if( c[ i ].s != -1 ){
						c[ i ].close = 1;
						rev = c[ i ].rev;
						if (rev){
							rev->handle(rev);
						}
					}
				}

				ws_worker_process_exit();
				
			}
			

			if( ws_terminate ){
				ws_worker_process_exit();
			}

			if( ws_quit ){
				ws_listen_close();
				if( !ws_exiting ){
					ws_exiting = 1;
				}
			}

			ws_cycling();

		}
}

static void
ws_worker_process_exit( )
{

    ws_mem_pool_free(ws_cycle->pool);


    exit(0);
}



ws_int32_t ws_os_child_process_init()
{
#ifdef __linux__
	sigset_t          set;
	ws_conn_t*		  c;
	ws_event_t*	  rev;

    if (ws_cycle->working_directory && ws_cycle->working_directory[0]) {
        if (chdir(ws_cycle->working_directory) == -1) {
            exit(2);
        }
    }



	sigemptyset(&set);

    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1) {
       printf( "sigprocmask fail\n" );
    }


	if( ws_cycle_process_init() ){
		return YMZ_ERROR;
	}
	

	c = ws_conn_get( ws_process_pipe.slot );
	if( !c ){
		return YMZ_ERROR;
	}

	rev = c->rev;

	rev->handle = ws_channel_handler;

	ws_event_add_conn( c );

#endif
	return YMZ_OK;


}

ws_int32_t ws_os_single_process_init()
{
    ws_process = YMZ_PROCESS_SINGLE;
#ifdef __linux__
	sigset_t          set;
	ws_conn_t*		  c;
	ws_event_t*	  rev;

    if (ws_cycle->working_directory && ws_cycle->working_directory[0]) {
        if (chdir(ws_cycle->working_directory) == -1) {
            exit(2);
        }
    }


	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, SIGALRM);
	sigaddset(&set, SIGIO);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGPIPE);


	if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
		printf("sigprocmask() failed");
	}

	sigemptyset(&set);

	/*if (sigprocmask(SIG_SETMASK, &set, NULL) == -1) {
	   printf( "sigprocmask fail\n" );
	   return YMZ_ERROR;
	}*/

	ws_process_id = 0;
	ws_process_fd = 0;
	ws_pid = getpid();

#endif
    
	if( ws_cycle_process_init() ){
		return YMZ_ERROR;
	}


	return YMZ_OK;
}

void ws_os_daemon_process()
{
#ifdef __linux__
	sigset_t						set;

	sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGIO);
    sigaddset(&set, SIGINT);
 

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        printf("sigprocmask() failed");
    }

    sigemptyset(&set);

	ws_process = YMZ_PROCESS_DAEMON;

	for( ;; ){
		  sigsuspend(&set);
		  ws_time_update();

		  if( ws_reap ){
			  ws_reap = 0;
		  }

		  if( ws_quit ){
			 exit( 2 );
		  }

	}

#endif

}