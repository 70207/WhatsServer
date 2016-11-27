#ifndef _WS_CORE_ADAPTER_H__
#define _WS_CORE_ADAPTER_H__



#include <ws_types.h>
#include <whatsserver.h>

ws_int32_t ws_core_adapter_init();

ws_int32_t ws_core_adapter_tcp_server_add(ws_server_tcp_t *server);
ws_int32_t ws_core_adapter_udp_server_add(ws_server_udp_t *server);

ws_int32_t ws_core_adapter_server_del(ws_int32_t server_index);


ws_int32_t ws_core_adapter_server_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t *itf);
ws_int32_t ws_core_adapter_server_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t *itf);


#endif