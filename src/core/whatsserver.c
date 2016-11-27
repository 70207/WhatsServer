#include "whatsserver.h"
#include <ws_core.h>


ws_int32_t ws_server_init(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path)
{
    return ws_core_server_init(working_path, log_path, lock_path, pid_path);
}

ws_int32_t ws_server_run()
{
    return ws_core_server_run();
}

//server
ws_int32_t ws_tcp_server_add(ws_server_tcp_t *server)
{
    return ws_core_tcp_server_add(server);
}
ws_int32_t ws_udp_server_add(ws_server_udp_t *server)
{
    return ws_core_udp_server_add(server);
}

ws_int32_t ws_server_del(ws_int32_t server_index)
{
    return ws_core_server_del(server_index);
}

//send
ws_int32_t ws_server_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf)
{
    return ws_core_server_tcp_send(server_index, conn_index, itf);
}

ws_int32_t ws_server_tcp_send_raw(ws_int32_t server_index, ws_int32_t conn_index, ws_char_t *data, ws_int32_t data_len)
{
    return YMZ_OK;
}
ws_int32_t ws_server_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t* itf)
{
    return ws_core_server_udp_send(server_index, ip, port, itf);
}