#include <ws_std.h>
#include <ws_inet.h>
#include <ws_event.h>
#include <ws_event_config.h>



ws_int32_t ws_event_process_init()
{
#ifdef __linux__
    ws_event_epoll_process_init();
#else
    ws_event_select_process_init();
#endif
	ws_event_timer_init();

	return YMZ_OK;

}

ws_int32_t ws_event_close()
{
#ifdef __linux__
    return ws_event_epoll_close();
#else
    return ws_event_select_close();
#endif
}

void ws_event_process_and_expire()
{
#ifdef __linux__
	ws_event_epoll_process();
#else
    ws_event_select_process();
#endif
	ws_event_timer_expire();
}


ws_int32_t ws_event_add( ws_event_t* ev, ws_int32_t type )
{
#ifdef __linux__
    return ws_event_epoll_add(ev, type);
#else
    return ws_event_select_add(ev, type);
#endif
}

ws_int32_t ws_event_add_conn( void* conn )
{
#ifdef __linux__
    return ws_event_epoll_add_conn(conn);
#else
    return ws_event_select_add_conn(conn);
#endif
}

ws_int32_t ws_event_del( ws_event_t* ev, ws_int32_t flags )
{
#ifdef __linux__
    return ws_event_epoll_del(ev, flags);
#else
    return ws_event_select_del(ev, flags);
#endif

}

ws_int32_t ws_event_del_conn( void* conn )
{
#ifdef __linux__
    return ws_event_epoll_del_conn(conn);
#else
    return ws_event_select_del_conn(conn);
#endif
}
