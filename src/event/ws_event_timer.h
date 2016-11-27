#ifndef _WS_EVENT_TIMER_H__
#define _WS_EVENT_TIMER_H__
#include <ws_event.h>


ws_int32_t ws_event_timer_init();
ws_int32_t ws_event_timer_add( ws_event_t* ev,  ws_msec_t timer );
ws_int32_t ws_event_timer_del( ws_event_t* ev );
time_t ws_event_timer_get();
void ws_event_timer_expire();







#endif //_WS_EVENT_TIMER_H__
