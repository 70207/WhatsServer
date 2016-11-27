#include <ws_std.h>
#include <ws_inet.h>

#include <ws_mem.h>
#include <ws_event.h>
#include <ws_conn.h>
#include <ws_conn_icmp.h>
#include <ws_cycle.h>
#include <ws_log.h>
#include <errno.h>

ws_int32_t ws_listen_tcp_listen_create(ws_ip_t  ip, ws_int32_t port, ws_listen_t** pplistener)
{
	ws_int32_t                   flag;
	ws_socket_t                  s;
	ws_int32_t                   size;
	struct sockaddr_in            addr;
	ws_mem_pool_t               *pool;
	ws_mem_buf_t                *buf;
	ws_listen_t                 *listener;
	ws_cycle_t                  *cycle;
    ws_int32_t                   error;


	cycle = ws_cycle;

	if( !cycle ){
		ws_log_sys_error( "cycle is null in function %s", __FUNCTION__ );
		return YMZ_CYCLE_ERROR;
	}

	size = sizeof( ws_listen_t );
    listener = ws_mem_malloc(size);
    if (!listener) {
        ws_log_sys_error("malloc mem for listen error!");
        return YMZ_CYCLE_ERROR;
    }

	s = socket( AF_INET, SOCK_STREAM, 0 );
	if( s <= 0 ){
		return YMZ_CYCLE_ERROR;
	}

	flag = ws_socket_nonblock( s );
	if( flag ){
		ws_log_sys_warn( "the listen socket cannot be set to nonblock" );
	}

	flag = ws_socket_reuse( s );
	if( flag ){
		ws_log_sys_warn( "the listen socket cannot be reused" );
	}

	addr.sin_family = AF_INET;
	addr.sin_port = port;
    addr.sin_addr.s_addr = ip;
	flag = bind( s, ( struct sockaddr* ) &addr, sizeof( addr ) );
    
	if( flag ){
        error = errno;
		close( s );
		return YMZ_CYCLE_ERROR;
	}

	flag = listen( s, cycle->listen_reserve );//cycle->server_listen_reserve );
    
	if( flag ){
		close( s );
		return YMZ_CYCLE_ERROR;
	}


	listener->s = s;
    listener->ip = ip;
	listener->port = port;
    

	*pplistener = listener;

	return YMZ_CYCLE_OK;
}

ws_int32_t ws_listen_udp_listen_create(ws_ip_t ip, ws_int32_t port, ws_listen_t** pplistener)
{
    ws_int32_t                   flag;
    ws_socket_t                  s;
    ws_int32_t                   size;
    struct sockaddr_in    addr;
    ws_mem_pool_t        *pool;
    ws_mem_buf_t         *buf;
    ws_listen_t          *listener;
    ws_cycle_t           *cycle;
        ws_int32_t                   error;
    
    cycle = ws_cycle;
    
    if( !cycle ){
        ws_log_sys_error( "cycle is null in function %s", __FUNCTION__ );
        return YMZ_CYCLE_ERROR;
    }
    
    size = sizeof(ws_listen_t);
    listener = ws_mem_malloc(size);
    if (!listener) {
        ws_log_sys_error("malloc mem for listen error!");
        return YMZ_CYCLE_ERROR;
    }
    
    s = socket( AF_INET, SOCK_DGRAM, 0 );
    if( s <= 0 ){
        return YMZ_CYCLE_ERROR;
    }
    
    flag = ws_socket_nonblock( s );
    if( flag ){
        ws_log_sys_warn( "the listen socket cannot be set to nonblock" );
    }
    
    flag = ws_socket_reuse( s );
    if( flag ){
        ws_log_sys_warn( "the listen socket cannot be reused" );
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = ip;
    flag = bind( s, ( struct sockaddr* ) &addr, sizeof( addr ) );
    
    if( flag ){
        error = errno;
        close( s );
        return YMZ_CYCLE_ERROR;
    }
    

    listener->s = s;
    listener->ip = ip;
	listener->port = port;
    
#define YMZ_UDP_BUF_MAX_SIZE     2048
    size = YMZ_UDP_BUF_MAX_SIZE;
    buf = ws_mem_malloc(size + sizeof(ws_mem_buf_t));
    if (!buf) {
        ws_log_sys_warn("mem error in function %s", __FUNCTION__);
        return YMZ_CYCLE_ERROR;
    }

    memset(buf, 0, size + sizeof(ws_mem_buf_t));
    buf->size = size;
    buf->hd = 0;
    buf->pos = 0;
    buf->data = ((ws_char_t*)buf) + sizeof(ws_mem_buf_t);
    buf->isneedfree = 1;
    
    listener->buffer = buf;
    
	*pplistener = listener;
    
    return YMZ_CYCLE_OK;
    

}

ws_int32_t ws_listen_icmp_listen_create(ws_listen_t** pplistener)
{
    ws_int32_t                   flag;
    ws_socket_t           s;
    ws_int32_t                   size;
    ws_listen_t         *listener;
    ws_cycle_t           *cycle;
    ws_mem_buf_t        *buf;

    cycle = ws_cycle;

    if (!cycle) {
        ws_log_sys_error("cycle is null in function %s", __FUNCTION__);
        return YMZ_CYCLE_ERROR;
    }

    size = sizeof(ws_listen_t);
  
    
    listener = ws_mem_malloc(size);
    if (!listener) {
        ws_log_sys_error("malloc mem for listen error!");
        return YMZ_CYCLE_ERROR;
    }

    s = ws_conn_icmp_socket_create();
    if (s <= 0) {
        return YMZ_CYCLE_ERROR;
    }

    flag = ws_socket_nonblock(s);
    if (flag) {
        ws_log_sys_warn("the listen socket cannot be set to nonblock");
    }

    flag = ws_socket_reuse(s);
    if (flag) {
        ws_log_sys_warn("the listen socket cannot be reused");
    }

    listener->s = s;


#define YMZ_UDP_BUF_MAX_SIZE     2048
    size = YMZ_UDP_BUF_MAX_SIZE;
   
    buf = ws_mem_malloc(size + sizeof(ws_mem_buf_t));
    if (!buf) {
        ws_log_sys_warn("mem error in function %s", __FUNCTION__);
        return YMZ_CYCLE_ERROR;
    }

    memset(buf, 0, size + sizeof(ws_mem_buf_t));
    buf->size = size;
    buf->hd = 0;
    buf->pos = 0;
    buf->data = ((ws_char_t*)buf) + sizeof(ws_mem_buf_t);
    buf->isneedfree = 1;

    listener->buffer = buf;


    *pplistener = listener;

    return YMZ_CYCLE_OK;


}


void ws_listen_free(ws_listen_t *ls)
{
    if (!ls) {
        return;
    }
    free(ls);
}


ws_int32_t ws_listen_cycle_init()
{
	

	return YMZ_CYCLE_OK;
}

ws_int32_t ws_listen_close()
{
	if( !ws_cycle ){
		ws_log_sys_error( "cycle is null in function %s", __FUNCTION__ );
		return YMZ_CYCLE_ERROR;
	}

	return YMZ_CYCLE_OK;
}


