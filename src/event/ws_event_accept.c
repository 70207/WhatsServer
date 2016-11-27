#include <ws_std.h>
#include <ws_inet.h>
#include <errno.h>


#include <ws_log.h>
#include <ws_mem.h>
#include <ws_time.h>
#include <ws_event.h>
#include <ws_event_timer.h>
#include <ws_conn.h>
#include <ws_net.h>
#include <ws_cycle.h>






ws_int32_t ws_event_accept( ws_event_t* ev )
{
	ws_conn_t* c;
	ws_mem_pool_t* pool;
	ws_mem_buf_t*  buf;
	ws_int32_t sock_fd;
	struct sockaddr_in  addr_in;
	ws_int32_t size;
	ws_int32_t avaliable;
	ws_socket_t s;

	ws_conn_t* lc;
	ws_listen_t* ls;

	if( !ev ){
		ws_log_error( "event is accept worng" );
		return YMZ_EVENT_ERROR;
	}




	avaliable = 1;

	lc = ev->data;
	ls = lc->listening;

	do{
		size = sizeof( struct sockaddr_in );
#ifdef __linux__
		s = accept4( ev->fd, ( struct sockaddr* )&addr_in, ( socklen_t* ) &size, SOCK_NONBLOCK );
#else
        s = accept( ev->fd, ( struct sockaddr* )&addr_in, ( socklen_t* ) &size );
#endif
		if( s == -1 ){
			switch( errno ){
				case EAGAIN:
					avaliable = 0;
					return YMZ_EVENT_EAGAIN;
				case EINTR:
				case ENOSYS:
					ws_log_sys_error( "nosys error in accept socket:%d, error:%d", ev->fd, errno );
				case ECONNABORTED:
					continue;
					break;
			}
			return YMZ_EVENT_ERROR;
		}

		ws_socket_nonblock( s );
		c = ws_conn_get( s );
		if( !c ){
			close( s );
			printf( "connot get conn in accept" );
			ws_log_error( "cannot get conn in accept" );
			return YMZ_EVENT_ERROR;
		}

		ws_log_access_info("accept a socket port:%lld", addr_in.sin_port );
		c->port = addr_in.sin_port;

		pool = c->pool;
		buf = ws_mem_buf_alloc( pool, size );
		if( !buf ){
			break;
		}

		c->sockaddr = buf->data;
		memcpy( c->sockaddr, &addr_in, size );
		c->life_time = ls->conn_life_time;
        c->tcp_server = ls->tcp_server;
        c->server_index = ls->server_index;
		ws_event_add_conn( c );

        ls->tcp_handle(c);
	
		
		
	}while( avaliable );

    return 0;

}

ws_int32_t ws_event_accept_udp( ws_event_t         *ev )
{
    ws_conn_t                               *c;
    ws_mem_buf_t                            *buf;
    struct sockaddr_in                       addr_in;
    ws_int32_t                                      size;
    ws_int32_t                                      len;
    ws_int32_t                                      avaliable;
    ws_int32_t                                      error;
	ws_msec_t								 current_accept_msec, current_handle_msec;
    
    ws_listen_t                            *ls;
    
    if( !ev ){
        ws_log_error( "event is accept udp worng" );
        return YMZ_EVENT_ERROR;
    }
    
    
    
    
    avaliable = 1;
    
    c = ev->data;
    ls = c->listening;
    buf = ls->buffer;


	current_accept_msec = ws_current_msec;

    
    do{
        len = sizeof( struct sockaddr_in );
        size =recvfrom(ev->fd, buf->data, buf->size, 0, (struct sockaddr*)&addr_in, &len);

        if(size <= 0){
            error = errno;
            avaliable = 0;
			break;
        }
        
        buf->hd = size;
		
        ls->udp_handle(ls, buf->data, size, addr_in.sin_addr.s_addr, addr_in.sin_port );
        
    }while( avaliable );


    return 0;
    

}
