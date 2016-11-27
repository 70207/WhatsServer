#include <ws_std.h>
#include <ws_inet.h>
#include <errno.h>


#include <ws_mem.h>
#include <ws_chain.h>
#include <ws_event.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_log.h>


#include <ws_conn_send.h>
#include <ws_conn_websocket.h>


ws_int32_t ws_conn_send_event( ws_event_t  *ev )
{
	ws_int32_t                   flag;
	ws_int32_t                   size;
	ws_int32_t                   pos;
	ws_int32_t					  xo;
	ws_int32_t                   error;
	ws_conn_t			 *c;


	c = ev->data;
	size = c->send_wpos;
	pos = c->send_pos;
	xo = size - pos;

    if( xo <= 0 ){
		c->send_pos = c->send_wpos = 0;
		ev->active = 0;
        return YMZ_OK;
    }


	while( pos < size ){
		flag = send( c->s, c->send_buf2+ pos, size - pos, 0 );
		if( flag < 0 ){
			error = errno;
			switch( error ){
				case EINTR:
					continue;
				case EAGAIN:
					return YMZ_AGAIN;
			}
			return YMZ_ERROR;
		}

		pos += flag;
		c->send_pos += flag;
	}

	ws_log_access_debug( "send buffer size:%d, index:%d", xo, c->index );
	if( c->send_wpos <= c->send_pos ){
		c->send_pos = c->send_wpos = 0;
		ev->active = 0;
	}

	return YMZ_OK;
}

ws_int32_t ws_conn_send_udp_index_empty(ws_socket_t s, ws_int32_t server_index, ws_mem_buf_t  *buffer)
{
    return 0;
}
ws_int32_t ws_conn_send_udp_addr(ws_socket_t s, ws_ip_t ip, ws_port_t port, ws_mem_buf_t  *buffer)
{
    ws_int32_t                   size;
    struct sockaddr_in    addr;
	ws_int32_t					  rtn;
    
    size = buffer->hd;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = port;
    
    
    rtn = sendto( s,  buffer->data, size, 0, (struct sockaddr*)&addr, sizeof(addr));
	if (rtn <= 0){
		ws_log_print_error("send to error:%d, addr:%s, port:%d, size:%d, s:%d\n", errno, 
            inet_ntoa(addr.sin_addr), addr.sin_port, size, s);
	}
    return YMZ_OK;
}

ws_int32_t ws_conn_send_udp_addr2(ws_socket_t s, void* paddr, ws_mem_buf_t  *buffer)
{
	ws_int32_t                   size;
	struct sockaddr_in   *addr;
	ws_int32_t					  rtn;


	size = buffer->hd;
	addr = paddr;


	rtn = sendto(s, buffer->data, size, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if (rtn <= 0){
        ws_log_print_error("send to error:%d, addr:%s, port:%d, size:%d, s:%d\n", errno,
            inet_ntoa(addr->sin_addr), addr->sin_port, size, s);
	}

	return YMZ_OK;
}

ws_int32_t ws_event_send(ws_event_t* ev)
{
	ws_conn_t*			c;
	ws_int32_t					rc;


	c = ev->data;

	rc = ws_conn_send_event(ev);

	if (rc == YMZ_ERROR){
        ws_log_print_debug("event send error, so close socket, conn:%d, port:%d",c->index, c->port);
		ws_conn_close(c);
		return rc;
	}
	else if (rc == YMZ_AGAIN){
		ws_event_add(ev, YMZ_EVENT_TYPE_WRITE);
		return rc;
	}
	else if (rc == YMZ_OK){
		if (c->send_wpos < c->send_pos){
			ws_log_error("wpos minus send pos, conn:%d, wpos:%d, pos:%d", c->index, c->send_wpos, c->send_pos);
		}
		else if (c->send_wpos == c->send_pos){
			ws_event_del(ev, 0);
		}
		else{
			ws_event_add(ev, YMZ_EVENT_TYPE_WRITE);
		}
	}


	return rc;
}



ws_int32_t ws_conn_tcp_check_send_buf(ws_conn_t *c, ws_mem_buf_t *buf)
{
    ws_int8_t											*write_pos;

    if (buf->hd <= 0) {
        buf->hd = 0;
        return YMZ_OK;
    }

    if (c->send_wpos + buf->hd >= YMZ_CONN_SEND_BUF_SIZE) {
        ws_log_print_debug("check send buf failed, conn:%d,send wpos:%d, need append:%d, max:%d", c->index, c->send_wpos, buf->hd, YMZ_CONN_SEND_BUF_SIZE);
        return YMZ_ERROR;
    }
    else {
        ws_log_debug("send buf conn:%d, wpos:%d, hd:%d",c->index, c->send_wpos, buf->hd);
    }

    write_pos = c->send_buf2 + c->send_wpos;
    memcpy(write_pos, buf->data, buf->hd);
   
    c->send_wpos += buf->hd;

    buf->hd = 0;

    return YMZ_OK;

}


ws_int32_t ws_conn_check_send_buf(ws_conn_t *c, ws_mem_buf_t *buf)
{
    switch (c->type) {
    case YMZ_CONN_TYPE_TCP:
        return ws_conn_tcp_check_send_buf(c, buf);
    case YMZ_CONN_TYPE_WEBSOCKET:
        return ws_conn_websocket_check_send_buf(c, buf);
    }

    return YMZ_OK;
}


ws_int32_t ws_conn_check_buf_and_send(ws_conn_t *c, ws_mem_buf_t *buf)
{
    if (!c || c->s <= 0) {
        ws_log_print_warn("ws_conn_check_buf_and_send error c is null!");
        return YMZ_ERROR;
    }
    if (YMZ_OK != ws_conn_check_send_buf(c, buf)) {
        return YMZ_ERROR;
    }
    ws_event_send(c->wev);
    return YMZ_OK;
}