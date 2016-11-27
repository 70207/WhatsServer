#ifndef _WS_EVENT_CONFIG_H__
#define _WS_EVENT_CONFIG_H__

#include <ws_event.h>

#ifdef __linux__
ws_int32_t ws_event_epoll_process_init();
ws_int32_t ws_event_epoll_close();

void ws_event_epoll_process();


ws_int32_t ws_event_epoll_add(ws_event_t* ev, ws_int32_t type);
ws_int32_t ws_event_epoll_del(ws_event_t* ev, ws_int32_t flags);
ws_int32_t ws_event_epoll_add_conn(void* conn);
ws_int32_t ws_event_epoll_del_conn(void* conn);
//ws_int32_t ws_event_mod( ws_event_t* ev, ws_int32_t type );

ws_int32_t ws_event_epoll_accept(ws_event_t* ev);
ws_int32_t ws_event_epoll_accept_udp(ws_event_t* ev);

#elif _WIN32

ws_int32_t ws_event_select_process_init();
ws_int32_t ws_event_select_close();

void ws_event_select_process();


ws_int32_t ws_event_select_add(ws_event_t* ev, ws_int32_t type);
ws_int32_t ws_event_select_del(ws_event_t* ev, ws_int32_t flags);
ws_int32_t ws_event_select_add_conn(void* conn);
ws_int32_t ws_event_select_del_conn(void* conn);
//ws_int32_t ws_event_mod( ws_event_t* ev, ws_int32_t type );

ws_int32_t ws_event_select_accept(ws_event_t* ev);
ws_int32_t ws_event_select_accept_udp(ws_event_t* ev);


#endif
#endif