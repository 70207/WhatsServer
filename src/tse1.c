#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <whatsserver.h>
#include <ws_log.h>

ws_int32_t tcp_recv_handle(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf)
{
    printf("recv tcp msg, server:%d,conn:%d, size:%d\n", server_index, conn_index, itf->size);
    return YMZ_OK;
}

ws_int32_t tcp_on_connect(ws_int32_t server_index, ws_int32_t conn_index)
{
    printf("on tcp connect, server:%d, conn:%d", server_index, conn_index);
    return YMZ_OK;
}

ws_int32_t tcp_on_closed(ws_int32_t server_index, ws_int32_t conn_index)
{
    printf("on tcp closed, server:%d, conn:%d", server_index, conn_index);
    return YMZ_OK;
}


ws_int32_t udp_recv_handle(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t* itf, ws_int32_t real_size)
{
    printf("recv udp msg, server:%d, size:%d\n", server_index, real_size);
    return YMZ_OK;
}

ws_server_tcp_t   ts;
ws_server_udp_t   us;



ws_int32_t main( ws_int32_t argc, ws_char_t** argv )
{
    ws_int32_t                             server_index;
    ws_server_init("", "", "", "");
    ws_log_set_path("");

    memset(&ts, 0, sizeof(ts));
    ts.ip = inet_addr("0.0.0.0");
    ts.port = htons(8080);
    ts.on_recv = tcp_recv_handle;
    ts.on_connect = tcp_on_connect;
    ts.on_closed = tcp_on_closed;

    ws_tcp_server_add(&ts);
    printf("create tcp server server_index:%d\n", ts.server_index);

    memset(&us, 0, sizeof(us));
    us.ip = inet_addr("0.0.0.0");
    us.port = htons(8080);
    us.on_recv = udp_recv_handle;
    ws_udp_server_add(&us);
    printf("create udp server server_index:%d\n", us.server_index);


    ws_server_run();
    while (1) {
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
        
    }
	return 0;
}