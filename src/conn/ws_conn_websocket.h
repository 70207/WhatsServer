#ifndef _WS_CONNN_WEBSOCKET_H__
#define _WS_CONNN_WEBSOCKET_H__

#include <ws_types.h>

ws_int32_t ws_conn_websocket_check_recv(ws_conn_t  *c);
ws_int32_t ws_conn_websocket_recv(ws_conn_t   *c);
ws_int32_t ws_conn_websocket_check_handle(ws_conn_t* c, ws_char_t** pos, ws_int32_t* packetsize);
ws_int32_t ws_conn_websocket_check_send_buf(ws_conn_t *c, ws_mem_buf_t *buf);

#endif