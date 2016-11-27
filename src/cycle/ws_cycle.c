#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ws_types.h>
#include <ws_cln_timer.h>
#include <ws_time.h>
#include <ws_mem.h>
#include <ws_event.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_process.h>
#include <ws_os.h>
#include <ws_net.h>
#include <ws_conf.h>
#include <ws_log.h>
#include <ws_cpu.h>










ws_int32_t ws_cycle_parse( ws_cycle_t* cycle);
	
ws_cycle_t* ws_cycle;



ws_int32_t ws_cycle_conf_parse(ws_cycle_t* cycle)
{
    cycle->event_reserve = 1024;
    cycle->conn_reserve = 1024;
    cycle->conn_buf_reserve = 0;
    cycle->conn_life_time = 10;
    cycle->listen_reserve = 1024;
    cycle->not_daemon = 1;
    cycle->worker_process_num = 1;
    cycle->working_directory = "/tmp";
    cycle->working_log_directory = "/tmp";
    cycle->working_lock_path = "/tmp";
    cycle->working_pid_path = "/tmp";
    cycle->log_mode = 1;
    cycle->cpu_affinity = 0;
    ws_log_set_mode(1);
    ws_log_set_level(1);
    
    return YMZ_CYCLE_OK;
}



ws_cycle_t* ws_cycle_create()
{
	ws_cycle_t* cycle;
	ws_mem_pool_t* pool;
	ws_mem_buf_t* buf;
	ws_int32_t size;

#define CYCLE_MEM_POOL_BLOCK_SIZE			( 1024 * 16 )
#define CYCLE_MEM_POOL_BLOCK_NUM			1024
	pool = ws_mem_pool_create( CYCLE_MEM_POOL_BLOCK_SIZE,
		CYCLE_MEM_POOL_BLOCK_NUM );
	if( !pool ){
		return 0;
	}

	size = sizeof( ws_cycle_t );
	buf = ws_mem_buf_alloc( pool, size );
	if( !buf ){
		ws_mem_pool_free( pool );
		return 0;
	}

	cycle = buf->data;
	memset( cycle, 0, size );
	cycle->pool = pool;
	return cycle;
}

ws_int32_t ws_cycle_parse( ws_cycle_t* cycle)
{
    return ws_cycle_conf_parse( cycle );
}


ws_int32_t ws_cycle_daemon_init( ws_cycle_t* cycle )
{
	ws_time_init();

	if( ws_conf_module_init( cycle ) ){
		return YMZ_CYCLE_ERROR;
	}

	if( ws_cycle_parse( cycle ) ){
		return YMZ_CYCLE_ERROR;
	}

	ws_cycle = cycle;

	return YMZ_CYCLE_OK;
	  
}

ws_int32_t ws_cycle_client_init( ws_cycle_t* cycle )
{
	ws_time_init();

	if( ws_conf_module_init( cycle ) ){
		return YMZ_CYCLE_ERROR;
	}

	if( ws_cycle_parse( cycle ) ){
		return YMZ_CYCLE_ERROR;
	}

	ws_cycle = cycle;

	return YMZ_CYCLE_OK;
}

ws_int32_t ws_cycle_init(ws_cycle_t *cycle)
{
	ws_time_init();

	if (ws_conf_module_init(cycle)){
		return YMZ_CYCLE_ERROR;
	}

	if (ws_cycle_parse(cycle)){
		return YMZ_CYCLE_ERROR;
	}

	ws_cycle = cycle;

	return YMZ_CYCLE_OK;
}

ws_int32_t ws_cycle_client_modules_init(ws_cycle_t *cycle)
{
    
	if (ws_conn_cycle_init()){
		printf("conn module init error\n");
		return YMZ_CYCLE_ERROR;
	}

	if (ws_log_set_path(cycle->working_directory)){
		printf("log init error\n");
		return YMZ_CYCLE_ERROR;
	}

	return YMZ_CYCLE_OK;
}

ws_int32_t ws_cycle_modules_init( ws_cycle_t* cycle )
{
	if( ws_listen_cycle_init() ){
		printf( "bind listen error\n" );
		return YMZ_CYCLE_ERROR;
	}
    	

	if( ws_conn_cycle_init() ){
		printf( "conn module init error\n" );
		return YMZ_CYCLE_ERROR;
	}


	if( ws_net_module_init() ){
		printf( "net module init error\n" );
		return YMZ_CYCLE_ERROR;
	}

	
	if( ws_log_set_path( cycle->working_log_directory ) ){
		printf( "log init error\n" );
		return YMZ_CYCLE_ERROR;
	}

	return YMZ_CYCLE_OK;
}

ws_int32_t ws_cycle_client_process_init()
{
	if (ws_event_process_init()){
		return YMZ_ERROR;
	}

	if (ws_cln_timer_init()){
		return YMZ_ERROR;
	}

	if (ws_chain_module_init()){
		return YMZ_ERROR;
	}

	return YMZ_OK;
}



ws_int32_t	ws_cycle_process_init()
{
    if (ws_cpu_setaffinity()) {
        return YMZ_ERROR;
    }
	if( ws_event_process_init() ){
		return YMZ_ERROR;
	}




	if( ws_cln_timer_init() ){
		return YMZ_ERROR;
	}


	/*if( ws_chain_module_init() ){
		return YMZ_ERROR;
	}*/


	return YMZ_OK;
}



ws_int32_t	ws_cycle_create_pidfile()
{
	FILE*			f;
	ws_int32_t				flag;
	char			buffer[256];
	

	if (!ws_cycle->working_pid_path){
		ws_cycle->working_pid_path = "/tmp/whatsserver.pid";
	}

	sprintf(buffer, "%s.%d", ws_cycle->working_pid_path, ws_process_id);
	f = fopen(buffer, "wt");
	if( !f ){
		return YMZ_ERROR;
	}

	fprintf( f, "%lld\n", ws_pid );

	fclose( f );
		

	return YMZ_OK;

}


void ws_cycling()
{
	ws_msec_t					current_msec;
	ws_time_update();

	current_msec = ws_current_msec;

	ws_event_process_and_expire();
	ws_cln_timer_expire();
}

void ws_client_cycling()
{
	ws_time_update();
	ws_event_process_and_expire();
	ws_cln_timer_expire();
}

ws_int32_t ws_cycle_path_set(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path)
{
    ws_mem_buf_t                                  *buf;
    ws_int32_t                                     size;


    size = strlen(working_path) + 1;
    buf = ws_mem_buf_alloc(ws_cycle->pool, size);
    if (!buf) {
        printf("buf alloc error!");
        return YMZ_ERROR;
    }

    memcpy(buf->data, working_path, size);
    ws_cycle->working_directory = buf->data;



    size = strlen(log_path) + 1;
    buf = ws_mem_buf_alloc(ws_cycle->pool, size);
    if (!buf) {
        printf("buf alloc error!");
        return YMZ_ERROR;
    }

    memcpy(buf->data, log_path, size);
    ws_cycle->working_log_directory = buf->data;


    size = strlen(lock_path) + 1;
    buf = ws_mem_buf_alloc(ws_cycle->pool, size);
    if (!buf) {
        printf("buf alloc error!");
        return YMZ_ERROR;
    }

    memcpy(buf->data, lock_path, size);
    ws_cycle->working_lock_path = buf->data;


    size = strlen(pid_path) + 1;
    buf = ws_mem_buf_alloc(ws_cycle->pool, size);
    if (!buf) {
        printf("buf alloc error!");
        return YMZ_ERROR;
    }

    memcpy(buf->data, pid_path, size);
    ws_cycle->working_pid_path = buf->data;

    return YMZ_OK;
}