##WhatsServer
What's a Server?
What things should a Server has?
----------------------------
WhatsServer is a server framework. with it you can build your own server fastly.
Right now ws(WhatsServer) support udp and tcp, on linux/windows/mac.

the api sample:
```c
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

```
as you will create a server with ws,

the first thing should be done is:
```c
ws_int32_t ws_server_init(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path);
```
which init the framework.

then you should set the log path, which can help you to diagnose. 

before you create a server, you should define the server with struct:
```c
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

``` 


then add server as below:

```c
ws_int32_t ws_tcp_server_add(ws_server_tcp_t *server);
ws_int32_t ws_udp_server_add(ws_server_udp_t *server);
```

after all, run the server immediately as you see.
```c
ws_int32_t ws_server_run();
```

