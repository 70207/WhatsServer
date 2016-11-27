#include <ws_std.h>
#include <ws_inet.h>

#include <ws_types.h>
#include <ws_mem.h>
#include <ws_time.h>
#include <ws_chain.h>
#include <ws_cycle.h>
#include <ws_event_timer.h>
#include <ws_conn.h>
#include <ws_conn_send.h>
#include <ws_log.h>
#include <ws_conn_send_chain.h>
#include <ws_conn_recv.h>
#include <ws_net.h>

ws_rbtree_t                        ws_net_server_tree;
ws_rbtree_node_t                   ws_net_server_sentinel;


ws_int32_t ws_net_udp_handle( ws_listen_t *l, ws_itf_header_t* itf, ws_int32_t size, ws_ip_t ip, ws_port_t port );



ws_int32_t ws_net_read( ws_event_t    *ev );
ws_net_t* ws_net_alloc( ws_conn_t* c );
ws_int32_t ws_net_do( ws_net_t* net );
ws_int32_t ws_net_event_empty_send( ws_event_t* ev );

ws_int32_t ws_net_block_read( ws_conn_t* c );
ws_int32_t ws_net_read_cycle( ws_event_t* ev );



ws_int32_t ws_net_module_init()
{
    ws_rbtree_init(&ws_net_server_tree, &ws_net_server_sentinel, ws_rbtree_insert_value);

	
	return YMZ_CYCLE_OK;

}



ws_int32_t ws_net_handle2(ws_conn_t  *c)
{
    ws_event_t* rev, *wev;

    rev = c->rev;
    wev = c->wev;

    ws_conn_nodelay(c->s);
    rev->handle = ws_net_read_cycle;
    wev->handle = ws_net_event_empty_send;


    c->tcp_server.on_connect(c->server_index, c->index);
    rev->handle(rev);

    return YMZ_CONN_OK;
}

ws_int32_t ws_net_block_read( ws_conn_t* c )
{
	ws_event_del( c->rev, 0 );
	return YMZ_CONN_OK;
}

#define YMZ_NR_OK	   0
#define YMZ_NR_AGAIN   1


ws_int32_t ws_net_read_cycle( ws_event_t* ev )
{
	ws_conn_t*							c;
	ws_int32_t									times;
	
	
	times = 1;
	c = ev->data;


	if (ev->timedout){
        ws_log_print_debug("net connection timed out:%d");
		ws_conn_close(c);
		return YMZ_OK;
	}

	ws_event_timer_add(ev, c->life_time);

	while( ( ws_net_read( ev ) == YMZ_NR_AGAIN ) && times ){
		times--;
		continue;
	}

	return YMZ_OK;
}

ws_int32_t ws_net_read( ws_event_t    *ev )
{
	ws_net_t*							net;
	ws_conn_t*							c;
	ws_mem_pool_t*						pool;
	ws_mem_buf_t*						buf;
	ws_clean_up_t*						cln;
	ws_int32_t									size;
	ws_int32_t									rtn;
	ws_int32_t									flag;
	ws_char_t*						        pos;
	ws_event_t*						wev;
	ws_int32_t									min;
	ws_int32_t									handled;
    ws_int32_t                                 hasClosed;
    ws_int32_t                                 lastRtn;
	
	
	c = ev->data;
	wev = c->wev;
	pool = c->pool;
	
	ws_log_access_debug("ws_net_read in, port:%d", c->port );


	if( ev->timedout || c->close ){
		ws_log_info("connection time out index:%d", c->index);
		ws_conn_close( c );
		return YMZ_NR_OK;
	}

    if (c->s <= 0) {
        ws_log_print_warn("net read error, fd:%d", c->s);
        return YMZ_ERROR;
    }
	
    hasClosed = 0;
	rtn = ws_check_recv( c );
	if( !rtn ){
		rtn = ws_conn_recv( c );
		switch( rtn )
		{
		case YMZ_CONN_RECV_ERROR:
			ws_log_error( "conn read fail, %llx\n", c );
            hasClosed = 1;
            lastRtn = YMZ_CONN_RECV_ERROR;
			break;
		case YMZ_CONN_CLIENT_CLOSE:
            ws_log_print_debug("read check error1, so close port:%d", c->port);
			c->pending_state = 0;
            hasClosed = 1;
			//ws_conn_close( c );
			lastRtn = YMZ_NR_OK;
            break;
		}
	}

	if( !hasClosed && c->recv_mark ){
		rtn = ws_check_recv( c );
		if( !rtn ){
			rtn = ws_conn_recv( c );
			switch( rtn )
			{
			case YMZ_CONN_RECV_ERROR:
				ws_log_error( "conn read fail, %llx\n", c );
                return YMZ_CONN_RECV_ERROR;
				break;
			case YMZ_CONN_CLIENT_CLOSE:
                ws_log_print_debug("read check error2, so close port:%d", c->port);
				c->pending_state = 0;
				ws_conn_close( c );
				return YMZ_NR_OK;
			}
		}
	}
	



	//-------------------------end of read--------------------//


	
	handled = 0;
	min = c->read_buffer_size - 512 - c->recv_pos;
	while( !ws_check_handle( c, &pos, &size ) ){
		handled++;
        c->tcp_server.on_recv(c->server_index, c->index, pos);
        if (c->s <= 0) {
            return YMZ_ERROR;
        }
	}

    if (hasClosed) {
        if (c->s > 0) {
            ws_conn_close(c);
        }
        return lastRtn;
    }

	if( c->send_wpos > c->send_pos ){
		wev->handle = ws_event_send;
	    ws_event_send( wev );
	}



	return YMZ_NR_OK;

}

ws_int32_t ws_net_udp_handle(ws_listen_t *l, ws_itf_header_t* itf, ws_int32_t size, ws_ip_t ip, ws_port_t port)
{
    l->udp_server.on_recv(l->server_index, ip, port, itf, size);

	return YMZ_OK;
}

ws_int32_t ws_net_send( ws_net_t* net )
{
	ws_conn_t* c;
	c = net->conn;
	return ws_event_send( c->wev );
}



ws_int32_t ws_net_event_empty_send( ws_event_t* ev )
{
	return YMZ_OK;
}

ws_int32_t ws_net_close( ws_net_t* net )
{


	if( !net ){
		return YMZ_NET_ERROR;
	}

	return 0;
}



ws_int32_t ws_net_tcp_server_add(ws_server_tcp_t *server)
{
    ws_listen_t                                *ls;
    ws_conn_t                                  *c;
    ws_event_t                                 *rev, *wev;
    ws_rbtree_key_t                             key;
    ws_rbtree_node_t                           *node;

    if (YMZ_OK != ws_listen_tcp_listen_create(server->ip, server->port, &ls)) {
        return YMZ_ERROR;
    }


    ls->conn_life_time = ws_cycle->conn_life_time * 1000;

    c = ws_conn_get(ls->s);
    if (c) {
        rev = c->rev;
        rev->handle = ws_event_accept;

        c->listening = ls;
        c->is_listen = 1;
        ls->c = c;

        ws_event_add(rev, YMZ_EVENT_TYPE_READ);
    }

    ls->tcp_handle = ws_net_handle2;
    ls->tcp_server = *server;
    ls->type = YMZ_SERVER_TYPE_TCP;
    ls->server_index = ls->s;

    key.v = ls->s;
    key.v2 = 0;

    node = &ls->node;
    node->key = key;

    ws_rbtree_insert(&ws_net_server_tree, node);

    ls->tcp_server.server_index = server->server_index = key.v;
    
    
    
    return YMZ_OK;
}

ws_int32_t ws_net_udp_server_add(ws_server_udp_t *server)
{
    ws_listen_t                                *ls;
    ws_conn_t                                  *c;
    ws_event_t                                 *rev, *wev;
    ws_rbtree_key_t                             key;
    ws_rbtree_node_t                           *node;

    if (YMZ_OK != ws_listen_udp_listen_create(server->ip, server->port, &ls)) {
        return YMZ_ERROR;
    }


    ls->conn_life_time = ws_cycle->conn_life_time * 1000;

    c = ws_conn_get(ls->s);
    if (c) {
        rev = c->rev;
        rev->handle = ws_event_accept_udp;

        c->listening = ls;
        c->is_listen = 1;
        ls->c = c;

        ws_event_add(rev, YMZ_EVENT_TYPE_READ);
    }

    ls->udp_handle = ws_net_udp_handle;
    ls->udp_server = *server;
    ls->type = YMZ_SERVER_TYPE_UDP;
    ls->server_index = ls->s;

    key.v = ls->s;
    key.v2 = 0;

    node = &ls->node;
    node->key = key;

    ws_rbtree_insert(&ws_net_server_tree, node);

    ls->tcp_server.server_index = server->server_index = key.v;


    return YMZ_OK;
}

ws_int32_t ws_net_tcp_server_add2(ws_listen_t  *listen)
{
    ws_listen_t                                *ls;
    ws_conn_t                                  *c;
    ws_event_t                                 *rev, *wev;
    ws_rbtree_key_t                             key;
    ws_rbtree_node_t                           *node;


    ls = listen;

    ls->conn_life_time = ws_cycle->conn_life_time * 1000;

    c = ws_conn_get(ls->s);
    if (c) {
        rev = c->rev;
        rev->handle = ws_event_accept;

        c->listening = ls;
        c->is_listen = 1;
        ls->c = c;

        ws_event_add(rev, YMZ_EVENT_TYPE_READ);
    }

    ls->tcp_handle = ws_net_handle2;
    ls->type = YMZ_SERVER_TYPE_TCP;

    key.v = ls->s;
    key.v2 = 0;

    node = &ls->node;
    node->key = key;

    ws_rbtree_insert(&ws_net_server_tree, node);

    return YMZ_OK;
}

ws_int32_t ws_net_udp_server_add2(ws_listen_t  *listen)
{
    ws_listen_t                                *ls;
    ws_conn_t                                  *c;
    ws_event_t                                 *rev, *wev;
    ws_rbtree_key_t                             key;
    ws_rbtree_node_t                           *node;

    ls = listen;

    ls->conn_life_time = ws_cycle->conn_life_time * 1000;

    c = ws_conn_get(ls->s);
    if (c) {
        rev = c->rev;
        rev->handle = ws_event_accept_udp;

        c->listening = ls;
        c->is_listen = 1;
        ls->c = c;

        ws_event_add(rev, YMZ_EVENT_TYPE_READ);
    }

    ls->udp_handle = ws_net_udp_handle;
    ls->type = YMZ_SERVER_TYPE_UDP;

    key.v = ls->s;
    key.v2 = 0;

    node = &ls->node;
    node->key = key;

    ws_rbtree_insert(&ws_net_server_tree, node);

    return YMZ_OK;
}

ws_int32_t ws_net_server_del(ws_int32_t server_index)
{
    ws_rbtree_key_t                             key;
    ws_rbtree_node_t                           *node;
    ws_listen_t                                *ls;
    ws_conn_t                                  *c;

    key.v = server_index;
    key.v2 = 0;

    if (YMZ_OK != ws_rbtree_get(&ws_net_server_tree, key, &ws_net_server_sentinel, &node)) {
        return YMZ_OK;
    }

    ws_rbtree_delete(&ws_net_server_tree, &node);

    ls = container_of(node, ws_listen_t, node);

    c = ls->c;

    ws_event_del_conn(c);
    ws_listen_free(ls);
    ws_conn_close(c);

    return YMZ_OK;

}

//send


ws_int32_t ws_net_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf)
{

    ws_conn_t                                  *c;
    ws_mem_buf_t                                buf;

    buf.data = itf;
    buf.size = buf.hd = itf->size;

    
    c = ws_conn_get2(conn_index);

    ws_conn_check_buf_and_send(c, &buf);

    return YMZ_OK;
}

ws_int32_t ws_net_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t* itf)
{
    ws_rbtree_key_t                             key;
    ws_rbtree_node_t                           *node;
    ws_listen_t                                *ls;
    ws_mem_buf_t                                buf;


    key.v = server_index;
    key.v2 = 0;

    if (YMZ_OK != ws_rbtree_get(&ws_net_server_tree, key, &ws_net_server_sentinel, &node)) {
        return YMZ_OK;
    }

    ls = container_of(node, ws_listen_t, node);

    buf.data = itf;
    buf.size = buf.hd = itf->size;

    return ws_conn_send_udp_addr(ls->s, ip, port, &buf);
}