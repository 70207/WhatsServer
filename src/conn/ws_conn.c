#include <ws_std.h>
#include <ws_inet.h>

#include <fcntl.h>

#include <ws_mem.h>
#include <ws_chain.h>
#include <ws_event.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_log.h>
#include <ws_conn_websocket.h>




ws_int32_t ws_conn_cycle_init()
{
	ws_cycle_t                 *cycle;
	ws_conn_t                  *c, *conn;
	ws_conn_t                  *next;
	ws_mem_pool_t              *pool;
	ws_mem_buf_t               *buf;
	ws_event_t                 *rev;
	ws_event_t                 *wev;


	ws_int32_t							i;
	ws_int32_t							size;
	ws_int32_t							reserve;

	cycle = ws_cycle;
	if( !cycle ){
		ws_log_sys_error( "conn init error for cycle is null" );
		return YMZ_CYCLE_ERROR;
	}

	reserve = cycle->conn_reserve;
	pool = cycle->pool;

	size = sizeof( ws_event_t ) * reserve;
	buf = ws_mem_buf_alloc( pool, size );
	if( !buf ){
			ws_log_sys_error( "conn alloc error" );
			return YMZ_CYCLE_ERROR;
	}
	rev = buf->data;
	memset( rev, 0, size );
	
	/*buf = ws_mem_buf_alloc( pool, size );
	if( !buf ){
			ws_log_sys_error( "conn alloc error" );
			return YMZ_CYCLE_ERROR;
	}
	wev = buf->data;*/
	wev = malloc(size);
	memset( wev, 0, size );


	size = sizeof( ws_conn_t ) * reserve;
    buf = ws_mem_buf_alloc( pool, size );
    if( !buf ){
        ws_log_sys_error( "conn alloc error" );
        return YMZ_CYCLE_ERROR;
    }
    
    conn = buf->data;
    memset( conn, 0, size );


	ws_cycle->conns = conn;
    
	next = 0;
	for( i = 0; i < reserve; ++i ){
		
		c = conn + i;
		c->index = i;
		rev->data = c;
		wev->data = c;

		c->rev = rev;
		c->wev = wev;
        
        c->next = cycle->free_conns;
        cycle->free_conns = c;

		rev += 1;
		wev += 1;

	}

	
	cycle->free_conn_num = reserve;

	cycle->conn_num = 0;

	return YMZ_CYCLE_OK;

}

ws_conn_t* ws_conn_get2(ws_int32_t index)
{
    return ws_cycle->conns + index;
}

ws_conn_t* ws_conn_get( ws_socket_t s )
{
	ws_conn_t             *c;
	ws_mem_pool_t         *pool;
	ws_mem_buf_t          *buf;
	ws_mem_buf_t          *recv_buf, *send_buf;
	ws_event_t            *rev, *wev;
    ws_int32_t             block_size;

	if( s <= 0 ){
		ws_log_sys_error( "conn cannot be get for socket is null" );
		return 0;
	}

	if( !ws_cycle->free_conns ){
		ws_log_sys_error( "conn pool is null" );
		return 0;
	}

#define YMZ_CONN_BLOCK_SIZE 40960
#define YMZ_CONN_BLOCK_NUM  4
    if (ws_cycle->conn_buf_reserve) {
        block_size = ws_cycle->conn_buf_reserve;
    }
    else {
        block_size = YMZ_CONN_BLOCK_SIZE;
    }
	pool = ws_mem_pool_create( block_size, YMZ_CONN_BLOCK_NUM );
	if( !pool ){
		ws_log_sys_error( "mem pool cannot be created" );
		return 0;
	}

	buf = ws_mem_buf_alloc( pool, block_size);
	if( !buf ){
        ws_log_print_error("get conn error, recv buf alloc failed!");
        ws_mem_pool_free( pool );
		return 0;
	}

	recv_buf = buf;

	buf = ws_mem_buf_alloc( pool, block_size);
	if( !buf ){
        ws_log_print_error("get conn error, send buf alloc failed!");
        ws_mem_pool_free( pool );
		return 0;
	}

	send_buf = buf;

	c = ws_cycle->free_conns;
	ws_cycle->free_conns = c->next;
	
	c->pool = pool;
	c->s = s;
	c->recv_buf = recv_buf;
	c->send_buf = send_buf;

	c->chain_num = 0;
	c->max_recv = 0;
	c->pending_state = 0;
	c->recv_mark = 0;
	c->last_finish = 0;

    c->linger = 0;

	c->read_buffer_size = c->write_buffer_size = YMZ_CONN_BLOCK_SIZE - 2048;
	c->handle_pos = 0;
	c->recv_pos = 0;
	c->user_data = 0;
    c->user_data2 = 0;

//-------------------------------------------this is for optimize

	buf = ws_mem_buf_alloc( pool, YMZ_CONN_SEND_BUF_SIZE );
	if( !buf ){
        ws_log_print_error("get conn error, send buf2 alloc failed!");
        ws_mem_pool_free( pool );
		return 0;
	}
	c->send_buf2 = buf->data;
	c->send_pos = 0;
	c->send_wpos = 0;
//---------

	rev = c->rev;
	wev = c->wev;

	rev->fd = s;
	wev->fd = s;

	rev->active = 0;
	rev->handle = 0;
	rev->type = 0;
	rev->timedout = 0;
	rev->timer_set = 0;


	wev->active = 0;
	wev->handle = 0;
	wev->type = 0;
	wev->timedout = 0;
	wev->timer_set = 0;

    c->type = YMZ_CONN_TYPE_TCP;

	ws_cycle->free_conn_num--;
	ws_cycle->conn_num++;

	ws_log_access_info("get a conn index:%d", c->index);
	return c;
}

ws_int32_t ws_conn_free( ws_conn_t *c )
{
	ws_int32_t							size;
	struct sockaddr_in*			addr;

	if( !c ){
		ws_log_error( "conn is error in function %s", __FUNCTION__ );
		return YMZ_CONN_ERROR;
	}

	addr = c->sockaddr;
	if (addr){
		ws_log_access_info("release a socket port:%lld, conn:%d", addr->sin_port, c->index);
	}

	ws_mem_pool_free( c->pool );
	ws_chain_free( c->send_chain );
    c->pool = 0;
    c->send_chain = 0;
	c->s = -1;
	c->start_time = c->expire_time = 0;
	c->recv_buf = c->send_buf = 0;
	c->send_wpos = 0;
	c->send_buf2 = 0;
    c->data = 0;
	c->user_data = 0;
    c->user_data2 = 0;
	
    c->linger = 0;

	c->send_chain = 0;

	c->next = ws_cycle->free_conns;
	ws_cycle->free_conns = c;
	ws_cycle->free_conn_num++;
	ws_cycle->conn_num--;

	return YMZ_CONN_OK;
}

ws_conn_t* ws_conn_udp_get( ws_int32_t ip, ws_int32_t port )
{
    ws_conn_t             *c;
    ws_int32_t                       s;
    struct sockaddr_in       *addr;
    ws_int32_t                       size;

    
	if( !ws_cycle->free_conns ){
		ws_log_error( "conn pool is null" );
		return 0;
	}
    
    s = socket( AF_INET, SOCK_DGRAM, 0 );
    if( s <= 0 ){
        return 0;
    }
    
    
    size = sizeof( struct sockaddr_in );
    addr = malloc( size );
    if( !addr ){
        close( s );
        return 0;
    }
    
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = ip;
    addr->sin_port = port;
    
	c = ws_cycle->free_conns;
	ws_cycle->free_conns = c->next;
	
    
    ws_socket_nonblock( s );
    bind( s, ( struct sockaddr* )addr, size );
    
	c->s = s;
    c->sockaddr = addr;
    c->type = YMZ_CONN_TYPE_UDP;
    
    ws_cycle->free_conn_num--;
	ws_cycle->conn_num++;
	return c;
}

ws_int32_t ws_conn_udp_close( ws_conn_t   *c )
{
    ws_int32_t                       s;
    struct sockaddr_in       *addr;
    ws_int32_t                       size;
    
	if (c->sockaddr){
		free(c->sockaddr);
	}
    close( c->s );
    
    c->s = -1;
	c->start_time = c->expire_time = 0;
	c->recv_buf = c->send_buf = 0;
    c->data = 0;
    
	c->next = ws_cycle->free_conns;
	ws_cycle->free_conns = c;
	ws_cycle->free_conn_num++;
	ws_cycle->conn_num--;
    
    return YMZ_CONN_OK;
    
}


ws_int32_t ws_socket_nonblock( ws_socket_t s )
{
#if defined(__linux__) || defined(__APPLE__)
	ws_int32_t flag = fcntl( s, F_GETFL, 0 );
	fcntl( s, F_SETFL, flag | O_NONBLOCK );

#else:
    unsigned long ul = 1;
    ioctlsocket(s, FIONBIO, (unsigned long *)&ul);
#endif
	return YMZ_CONN_OK;
}

ws_int32_t ws_socket_reuse( ws_socket_t s )
{
    ws_int32_t reuseaddr = 1;
    setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof( ws_int32_t ) );

	return YMZ_CONN_OK;
}

ws_int32_t ws_conn_close( ws_conn_t *c )
{
	ws_clean_up_t*		cln;
	ws_event_t*			rev;
	ws_event_t*			wev;
    struct linger           lgv;


	if( c->s == -1 ){
		ws_log_access_error("close conn:%d, but the socket is error, user data:%d, user data2:%d",
            c->index, c->user_data, c->user_data2);
		return YMZ_CONN_ERROR;
	}
    c->tcp_server.on_closed(c->server_index, c->index);

	rev = c->rev;
	wev = c->wev;



	if( rev->timer_set ){
		ws_event_timer_del( rev );
	}

	if( wev->timer_set ){
		ws_event_timer_del( wev );
	}

	ws_event_del_conn( c );

	rev->active = 0;
	wev->active = 0;

	while( c->clean_up ){
		cln = c->clean_up;
        if (cln->clean_up) {
            cln->clean_up(cln->data);
        }
        if (cln->clean_up2) {
            cln->clean_up2(cln->data, cln->data2);
        }

		if (c->clean_up == cln){                            //可能会产生回环
			c->clean_up = cln->next;
		}

	}

	c->clean_up = 0;

    if (c->linger) {
        lgv.l_onoff = 1;
        lgv.l_linger = 1;

        setsockopt(c->s, SOL_SOCKET, SO_LINGER, &lgv, sizeof(struct linger));
    }

    close( c->s );

	ws_log_access_info("close conn:%d, socket:%d ", c->index, c->s);

	ws_conn_free( c );


	return 0;
}


void ws_conn_add_clean_up( ws_conn_t* c, ws_clean_up_t* cln )
{
	ws_clean_up_t*			cln2;

	cln2 = c->clean_up;
	if( cln2 ){
		if (cln2 == cln){
			return;
		}
		cln2->prev = cln;
	}

	cln->next = cln2;
	cln->prev = 0;
	c->clean_up = cln;
}


void ws_conn_del_clean_up( ws_conn_t* c,  ws_clean_up_t* cln )
{
	ws_clean_up_t*			cln2;
    if (!c->clean_up) {
        ws_log_print_warn("conn del clean up but clean up error, conn:%d, clean up:%d, s:%d", c->index, c->clean_up, c->s);
        return;
    }
	if( cln->prev ){
		cln2 = cln->prev;
		cln2->next = cln->next;
	}
	else{
		c->clean_up = cln->next;
	}

	if( cln->next ){
		cln2 = cln->next;
		cln2->prev = cln->prev;
	}

}


void ws_conn_cork( ws_socket_t s )
{
	ws_int32_t on;
	on = 1;
#if defined(__linux__)
    setsockopt( s, SOL_TCP, TCP_CORK, &on, sizeof (on));
#else
   // setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
#endif
}

void ws_conn_ncork( ws_socket_t s )
{
	ws_int32_t on;
	on = 0;
#if defined(__linux__)
    setsockopt(s, SOL_TCP, TCP_CORK, &on, sizeof(on));
#else
    // setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
#endif
}

void ws_conn_nodelay( ws_socket_t s )
{
	ws_int32_t on;
	on = 1;

#if defined(__linux__)
    setsockopt(s, SOL_TCP, TCP_NODELAY, &on, sizeof(on));
#elif defined(__APPLE__)
    setsockopt(s, SOL_SOCKET, TCP_NODELAY, &on, sizeof(on));
#else
     setsockopt(s, SOL_SOCKET, TCP_NODELAY, &on, sizeof(on));
#endif
}

ws_int32_t  ws_conn_socket_udp_create()
{
	return socket(AF_INET, SOCK_DGRAM, 0);
}

