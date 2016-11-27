#include <ws_std.h>
#include <ws_inet.h>
#include <errno.h>



#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <ws_types.h>
#include <ws_socket.h>




ws_int32_t      ws_ntop(ws_ip_t  ip, ws_int8_t  *buf)
{
    struct in_addr                                           sa;
    sa.s_addr = ip;
    inet_ntop(AF_INET, &sa, buf, INET_ADDRSTRLEN);

    return YMZ_OK;
}


ws_socket_t      ws_tcp_socket_create()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

ws_socket_t      ws_udp_socket_create()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}