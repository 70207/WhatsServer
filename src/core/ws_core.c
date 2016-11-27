#include <ws_std.h>


#include <ws_mem.h>
#include <ws_event.h>
#include <ws_time.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_log.h>
#include <ws_daemon.h>
#include <ws_process.h>
#include <ws_os.h>
#include <ws_core_adapter.h>



#include <ws_net.h>
#include <whatsserver.h>
#include <ws_cycle.h>

static ws_int32_t  ws_started = 0;

ws_int32_t ws_core_server_init(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path)
{
    ws_cycle_t*		cycle;

#ifdef _WIN32
    setvbuf(stdout, 0, _IONBF, 0);
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }

#else
    setbuf(stdout, 0);
#endif


    ws_log_init();

    if ((cycle = ws_cycle_create()) == 0) {
        return YMZ_ERROR;
    }


    if (ws_cycle_init(cycle)) {
        return YMZ_ERROR;
    }

    ws_cycle_path_set(working_path, log_path, lock_path, pid_path);

    if (ws_started_check()) {
        printf("you have started a program!\n");
        return 0;
    }

    if (ws_cycle_modules_init(cycle)) {
        return YMZ_ERROR;
    }

    ws_init_signals();
    if (!cycle->not_daemon) {
        ws_daemon();
    }

    if (ws_cycle_create_pidfile()) {
        return YMZ_ERROR;
    }

  
    if (ws_cycle->worker_process_num <= 1) {
        if (ws_os_single_process_init()) {
            return YMZ_ERROR;
        }
    }

    ws_core_adapter_init();
    
    return YMZ_OK;
}


ws_int32_t ws_core_server_run()
{
    ws_started = 1;
    if (ws_cycle->worker_process_num <= 1) {
        ws_os_single_process();
    }
    else
    {
        ws_os_master_process();
    }

    return YMZ_OK;
}




ws_int32_t ws_core_path_set(const ws_char_t *working_path, const ws_char_t *log_path,
    const ws_char_t *lock_path, const ws_char_t *pid_path)
{
    return ws_cycle_path_set(working_path, log_path, lock_path, pid_path);
}
//server
ws_int32_t ws_core_tcp_server_add(ws_server_tcp_t *server)
{
    if (!ws_started) {
        return ws_net_tcp_server_add(server);
    }
    return ws_core_adapter_tcp_server_add(server);
}
ws_int32_t ws_core_udp_server_add(ws_server_udp_t *server)
{
    if (!ws_started) {
        return ws_net_udp_server_add(server);
    }
    return ws_core_adapter_udp_server_add(server);
}

ws_int32_t ws_core_server_del(ws_int32_t server_index)
{
    return ws_core_adapter_server_del(server_index);
}

//send
ws_int32_t ws_core_server_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf)
{
    return ws_core_adapter_server_tcp_send(server_index, conn_index, itf);
}
ws_int32_t ws_core_server_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port, ws_itf_header_t* itf)
{
    return ws_core_adapter_server_udp_send(server_index, ip, port, itf);
}