#ifndef _WS_CONN_SEND_H__
#define _WS_CONN_SEND_H__

#define YMZ_CONN_SEND_OK		    0
#define YMZ_CONN_SEND_ERROR      1
#define YMZ_CONN_SEND_AGAIN	    2

#include <ws_types.h>
#include <ws_mem.h>
ws_int32_t ws_conn_send_event( ws_event_t  *ev );

ws_int32_t ws_conn_send_udp_index_empty(ws_socket_t  s, ws_int32_t server_index, ws_mem_buf_t  *buffer);
ws_int32_t ws_conn_send_udp_addr(ws_socket_t s, ws_ip_t ip, ws_port_t port, ws_mem_buf_t  *buffer);
ws_int32_t ws_conn_send_udp_addr2(ws_socket_t s, void* paddr, ws_mem_buf_t  *buffer);
ws_int32_t ws_event_send(ws_event_t* ev);

ws_int32_t ws_conn_check_send_buf(ws_conn_t *c, ws_mem_buf_t *buf);
ws_int32_t ws_conn_check_buf_and_send(ws_conn_t *c, ws_mem_buf_t *buf);

#endif
