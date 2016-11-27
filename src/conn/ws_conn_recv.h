#ifndef _WS_CONN_RECV_H__
#define _WS_CONN_RECV_H__


ws_int32_t ws_conn_recv( ws_conn_t  *c );
ws_int32_t ws_conn_recv_init( ws_conn_t *c );

ws_int32_t ws_check_recv( ws_conn_t  *c );
ws_int32_t ws_check_handle( ws_conn_t* c, ws_char_t** pos, ws_int32_t* psize );



#endif //_WS_CONN_RECV_H__
