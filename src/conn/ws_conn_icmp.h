#ifndef _WS_CONNN_ICMP_H__
#define _WS_CONNN_ICMP_H__

#include <ws_types.h>

typedef struct ws_conn_icmp_s           ws_conn_icmp_t;

struct ws_conn_icmp_s
{
    ws_int32_t                      ip;
    ws_int32_t                      port;

    ws_int32_t                      seq_id;
    ws_int32_t                      data_len;
    ws_int8_t                      *data;
};

ws_int32_t ws_conn_icmp_socket_create();

ws_int32_t ws_conn_send_icmp_echo(ws_socket_t s, ws_conn_icmp_t  *t);

ws_int32_t ws_conn_recv_icmp(ws_int8_t *packet, ws_int32_t p_size, ws_conn_icmp_t  *t);




#endif
