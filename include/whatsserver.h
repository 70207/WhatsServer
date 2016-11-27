#ifndef _WS_H__
#define _WS_H__


#include "ws_types.h"

typedef struct ws_server_tcp_s  ws_server_tcp_t;
typedef struct ws_server_udp_s  ws_server_udp_t;

#define YMZ_SERVER_TYPE_TCP  1
#define YMZ_SERVER_TYPE_UDP  2

#define YMZ_SERVER_DATA_TYPE_ITF     1
#define YMZ_SERVER_DATA_TYPE_RAW     0

struct ws_server_tcp_s
{
    ws_int32_t server_index;
    ws_int32_t server_type;
    ws_int32_t data_type;
    ws_ip_t    ip;
    ws_port_t  port;
    ws_int32_t (*on_recv)(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf);
    ws_int32_t (*on_connect)(ws_int32_t server_index, ws_int32_t conn_index);
    ws_int32_t (*on_closed)(ws_int32_t server_index, ws_int32_t conn_index);
    ws_int32_t (*on_error)(ws_int32_t server_index, ws_int32_t conn_index, ws_int32_t error);
    ws_int32_t (*on_raw_recv)(ws_int32_t server_index, ws_int32_t conn_index, ws_char_t *data, ws_char_t data_len);
};


struct ws_server_udp_s
{
    ws_int32_t server_index;
    ws_int32_t server_type;
    ws_ip_t    ip;
    ws_port_t  port;
    ws_int32_t (*on_recv)(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t* itf, ws_int32_t real_size);
    ws_int32_t (*on_raw_recv)(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_char_t *data, ws_int32_t data_len);
};


ws_int32_t ws_server_init(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path);

ws_int32_t ws_server_run();


//server
ws_int32_t ws_tcp_server_add(ws_server_tcp_t *server);
ws_int32_t ws_udp_server_add(ws_server_udp_t *server);
ws_int32_t ws_server_del(ws_int32_t server_index);

//send
ws_int32_t ws_server_tcp_send_raw(ws_int32_t server_index, ws_int32_t conn_index, ws_char_t *data, ws_int32_t data_len);
ws_int32_t ws_server_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf);
ws_int32_t ws_server_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port,  ws_itf_header_t* itf);




#endif