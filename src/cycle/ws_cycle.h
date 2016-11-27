#ifndef __ws_CONF_H__
#define __ws_CONF_H__

#include <ws_types.h>
#include <ws_mem.h>
#include <ws_conn.h>


typedef struct ws_cycle_s ws_cycle_t;


#define YMZ_CYCLE_ERROR -1
#define YMZ_CYCLE_OK 0



struct ws_cycle_s
{
	ws_int32_t						event_reserve;
	ws_int32_t						conn_reserve;
    ws_int32_t                     conn_buf_reserve;
	ws_int32_t						conn_life_time;
	ws_int32_t						listen_reserve;
	ws_int32_t						not_daemon;

	ws_int32_t						worker_process_num;
	const ws_char_t*					    working_directory;
	const ws_char_t*					    working_log_directory;
	const ws_char_t*					    working_lock_path;
	const ws_char_t*					    working_pid_path;

    ws_int32_t                     log_mode;

	
    ws_int32_t                     cpu_affinity;
	ws_mem_pool_t*			pool;


	ws_conn_t*				free_conns;
	ws_conn_t*				conns;
	

	ws_int32_t						conn_num;
	ws_int32_t						free_conn_num;
	ws_int32_t						quit;


	ws_int32_t						debug_level;
};
ws_cycle_t* ws_cycle_create();
ws_int32_t ws_cycle_init( ws_cycle_t* cycle );
ws_int32_t ws_cycle_modules_init(ws_cycle_t* cycle);
ws_int32_t	ws_cycle_process_init();


ws_int32_t ws_cycle_client_modules_init(ws_cycle_t *cycle);
ws_int32_t ws_cycle_client_process_init();

ws_int32_t ws_cycle_daemon_init( ws_cycle_t* cycle );
ws_int32_t ws_cycle_client_init( ws_cycle_t* cycle );
ws_int32_t	ws_cycle_create_pidfile();	

void ws_cycling();
void ws_client_cycling();


ws_int32_t ws_cycle_path_set(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path);


extern ws_cycle_t* ws_cycle;


#endif
