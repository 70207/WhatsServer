#include <ws_inet.h>
#include <ws_core_adapter.h>
#include <ws_socket.h>
#include <ws_conn.h>
#include <ws_conn_send.h>
#include <ws_event.h>
#include <ws_net.h>
#include <ws_log.h>

#define YMZ_ADAPTER_IP              "127.0.0.1"
#define YMZ_ADAPTER_PORT            9030

static ws_socket_t             ws_adapter_inner_s;
static ws_socket_t             ws_adapter_ourter_s;

static ws_ip_t                 ws_adapter_ip;
static ws_port_t               ws_adapter_port;


typedef struct ws_itf_adapter_server_s    ws_itf_adapter_server_t;
typedef struct ws_itf_adapter_send_s       ws_itf_adapter_send_t;

#define YMZ_ITF_ADAPTER_CMD_SERVER_ADD               1
#define YMZ_ITF_ADAPTER_CMD_SERVER_DEL               2
#define YMZ_ITF_ADAPTER_CMD_SEND_TCP                 3
#define YMZ_ITF_ADAPTER_CMD_SEND_UDP                 4

struct ws_itf_adapter_server_s
{
    ws_itf_header_t        header;
    ws_int32_t             type;
    ws_listen_t           *listen;
    ws_int32_t             server_index;
};

struct ws_itf_adapter_send_s
{
    ws_itf_header_t        header;
    ws_int32_t             server_index;
    ws_ip_t                dest_ip;
    ws_port_t              dest_port;
    ws_int32_t             dest_index;
    ws_int32_t             size;
    ws_char_t              data[0];
};


ws_int32_t ws_core_adapter_handle(ws_listen_t *l, ws_itf_header_t* itf, ws_int32_t size, ws_ip_t ip, ws_port_t port);
ws_int32_t ws_core_adapter_handle_internal(ws_listen_t *l, ws_itf_header_t* itf, ws_int32_t size, ws_ip_t ip, ws_port_t port);

ws_int32_t ws_core_adapter_init()
{
    ws_socket_t                                 s;
    ws_int32_t                                  flag;
    struct sockaddr_in                           addr;
    ws_listen_t                                *ls;
    ws_conn_t                                  *c;
    ws_event_t                                 *rev, *wev;



    //inner socket
    if (YMZ_OK != ws_listen_udp_listen_create(YMZ_ADAPTER_IP, YMZ_ADAPTER_PORT, &ls)) {
        return YMZ_ERROR;
    }

    ws_adapter_ip = inet_addr(YMZ_ADAPTER_IP);
    ws_adapter_port = htons(YMZ_ADAPTER_PORT);
    //outer socket

    ws_adapter_inner_s = ls->s;

    s = ws_udp_socket_create();

    flag = ws_socket_nonblock(s);
    if (flag) {
        ws_log_sys_warn("the adapter socket cannot be set to nonblock");
    }

    flag = ws_socket_reuse(s);
    if (flag) {
        ws_log_sys_warn("the listen socket cannot be reused");
    }

    ws_adapter_ourter_s = s;


    ls->conn_life_time = 0;

    c = ws_conn_get(ls->s);
    if (c) {
        rev = c->rev;
        rev->handle = ws_event_accept_udp;

        c->listening = ls;
        c->is_listen = 1;
        ls->c = c;

        ws_event_add(rev, YMZ_EVENT_TYPE_READ);
    }

    ls->udp_handle = ws_core_adapter_handle;
    ls->udp_server.on_recv = ws_core_adapter_handle_internal;
    ls->type = YMZ_SERVER_TYPE_UDP;

    return YMZ_OK;
}


ws_int32_t ws_core_adapter_handle_server_add(ws_itf_header_t* itf, ws_int32_t size)
{
    ws_itf_adapter_server_t                         *req;
    ws_listen_t                                     *ls;
    req = itf;
    ls = req->listen;

    switch (req->type) {
    case YMZ_SERVER_TYPE_TCP:
        ws_net_tcp_server_add2(ls);
        break;
    case YMZ_SERVER_TYPE_UDP:
        ws_net_udp_server_add2(ls);
    }
    
    return YMZ_OK;
}


ws_int32_t ws_core_adapter_handle_server_del(ws_itf_header_t* itf, ws_int32_t size)
{
    ws_itf_adapter_server_t                         *req;
    req = itf;
    
    ws_net_server_del(req->server_index);

    return YMZ_OK;
}

ws_int32_t ws_core_adapter_handle_send_tcp(ws_itf_header_t* itf, ws_int32_t size)
{
    ws_itf_adapter_send_t                             *req;
    ws_itf_header_t                                   *data;
    req = itf;
    data = req->data;

    return ws_net_tcp_send(req->server_index, req->dest_index, data);
}


ws_int32_t ws_core_adapter_handle_send_udp(ws_itf_header_t* itf, ws_int32_t size)
{
    ws_itf_adapter_send_t                             *req;
    ws_itf_header_t                                   *data;
    req = itf;
    data = req->data;

    return ws_net_udp_send(req->server_index, req->dest_ip, req->dest_port, data);
}

ws_int32_t ws_core_adapter_handle_internal(ws_listen_t *l, ws_itf_header_t* itf, ws_int32_t size, ws_ip_t ip, ws_port_t port)
{
    switch (itf->cmd) {
    case YMZ_ITF_ADAPTER_CMD_SERVER_ADD:
        ws_core_adapter_handle_server_add(itf, size);
        break;
    case YMZ_ITF_ADAPTER_CMD_SERVER_DEL:
        ws_core_adapter_handle_server_del(itf, size);
        break;
    case YMZ_ITF_ADAPTER_CMD_SEND_TCP:
        ws_core_adapter_handle_send_tcp(itf, size);
        break;
    case YMZ_ITF_ADAPTER_CMD_SEND_UDP:
        ws_core_adapter_handle_send_udp(itf, size);
        break;
    }

    return YMZ_OK;
}

ws_int32_t ws_core_adapter_handle(ws_listen_t *l, ws_itf_header_t* itf, ws_int32_t size, ws_ip_t ip, ws_port_t port)
{

    l->udp_server.on_recv(l->server_index, ip, port, itf, size);

    return YMZ_OK;
}


ws_int32_t ws_core_adapter_tcp_server_add(ws_server_tcp_t *server)
{
    ws_listen_t                                *ls;
    ws_itf_adapter_server_t                     req;
    ws_itf_header_t                            *header;
    ws_mem_buf_t                                buf;
    

    header = &req.header;

    header->cmd = YMZ_ITF_ADAPTER_CMD_SERVER_ADD;

    if (YMZ_OK != ws_listen_tcp_listen_create(server->ip, server->port, &ls)) {
        return YMZ_ERROR;
    }

    ls->server_index = ls->s;
    server->server_index = ls->s;
    ls->tcp_server = *server;
    

    server->server_index = ls->s;
    req.listen = ls;
    req.type = YMZ_SERVER_TYPE_TCP;

    buf.data = header;
    buf.size = sizeof(ws_itf_adapter_server_t);
    buf.hd = buf.size;

    ws_conn_send_udp_addr(ws_adapter_ourter_s, ws_adapter_ip, ws_adapter_port, &buf);

    return YMZ_OK;
}

ws_int32_t ws_core_adapter_udp_server_add(ws_server_udp_t *server)
{
    ws_listen_t                                *ls;
    ws_itf_adapter_server_t                     req;
    ws_itf_header_t                            *header;
    ws_mem_buf_t                                buf;

    header = &req.header;

    header->cmd = YMZ_ITF_ADAPTER_CMD_SERVER_ADD;

    if (YMZ_OK != ws_listen_tcp_listen_create(server->ip, server->port, &ls)) {
        return YMZ_ERROR;
    }

    ls->server_index = ls->s;
    server->server_index = ls->s;
    ls->udp_server = *server;

    server->server_index = ls->s;
    req.listen = ls;
    req.type = YMZ_SERVER_TYPE_UDP;

    buf.data = header;
    buf.size = sizeof(ws_itf_adapter_server_t);
    buf.hd = buf.size;

    ws_conn_send_udp_addr(ws_adapter_ourter_s, ws_adapter_ip, ws_adapter_port, &buf);

    return YMZ_OK;
}

ws_int32_t ws_core_adapter_server_del(ws_int32_t server_index)
{
    ws_itf_adapter_server_t                     req;
    ws_itf_header_t                            *header;
    ws_mem_buf_t                                buf;

    header = &req.header;

    header->cmd = YMZ_ITF_ADAPTER_CMD_SERVER_DEL;

   
    req.server_index = server_index;

    buf.data = header;
    buf.size = sizeof(ws_itf_adapter_server_t);
    buf.hd = buf.size;

    ws_conn_send_udp_addr(ws_adapter_ourter_s, ws_adapter_ip, ws_adapter_port, &buf);

    return YMZ_OK;
}



ws_int32_t ws_core_adapter_server_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t *itf)
{
    ws_itf_adapter_send_t                      *req;
    ws_itf_header_t                            *header;
    ws_char_t                                   buffer[40960];
    ws_mem_buf_t                                buf;

    req = buffer;
    header = &req->header;

    header->cmd = YMZ_ITF_ADAPTER_CMD_SEND_TCP;


    req->server_index = server_index;
    req->dest_index = conn_index;
    memcpy(req->data, itf, itf->size);
    req->size = itf->size;

    buf.data = header;
    buf.size = sizeof(ws_itf_adapter_send_t) + itf->size;
    buf.hd = buf.size;

    ws_conn_send_udp_addr(ws_adapter_ourter_s, ws_adapter_ip, ws_adapter_port, &buf);

    return YMZ_OK;
}

ws_int32_t ws_core_adapter_server_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t *itf)
{
    ws_itf_adapter_send_t                      *req;
    ws_itf_header_t                            *header;
    ws_char_t                                   buffer[40960];
    ws_mem_buf_t                                buf;

    req = buffer;
    header = &req->header;

    header->cmd = YMZ_ITF_ADAPTER_CMD_SEND_UDP;


    req->server_index = server_index;
    req->dest_ip = ip;
    req->dest_port = port;
    memcpy(req->data, itf, itf->size);
    req->size = itf->size;

    buf.data = header;
    buf.size = sizeof(ws_itf_adapter_send_t) + itf->size;
    buf.hd = buf.size;

    ws_conn_send_udp_addr(ws_adapter_ourter_s, ws_adapter_ip, ws_adapter_port, &buf);

    return YMZ_OK;
}