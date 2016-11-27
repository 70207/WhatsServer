#ifndef _WS_CORE_H__
#define _WS_CORE_H__

#include <ws_types.h>

ws_int32_t ws_core_server_init(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path);


ws_int32_t ws_core_server_run();

ws_int32_t ws_core_path_set(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path);

//server
ws_int32_t ws_core_tcp_server_add(ws_server_tcp_t *server);
ws_int32_t ws_core_udp_server_add(ws_server_udp_t *server);
ws_int32_t ws_core_server_del(ws_int32_t server_index);

//send
ws_int32_t ws_core_server_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf);
ws_int32_t ws_core_server_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t* itf);











#endif
