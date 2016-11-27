#ifndef _WS_SOCKET_H__
#define _WS_SOCKET_H__

#include <ws_types.h>


ws_int32_t      ws_ntop(ws_ip_t  ip, ws_int8_t  *buf);


ws_socket_t      ws_tcp_socket_create();
ws_socket_t      ws_udp_socket_create();




#endif