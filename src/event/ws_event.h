#ifndef _WS_EVENT_H__
#define _WS_EVENT_H__

#include <ws_rbtree.h>

typedef struct ws_event_s ws_event_t;



#define YMZ_EVENT_OK						0
#define YMZ_EVENT_ERROR					1
#define YMZ_EVENT_NO_EVENTS				2
#define YMZ_EVENT_EAGAIN					3

#define YMZ_READ							1
#define YMZ_WRITE						2
#define YMZ_ET							8
#define YMZ_EVENT_CLOSE					16
#define YMZ_EVENT_OUTLAW					17



typedef ws_int32_t (*ws_event_handle_ptr)( ws_event_t* ev );

struct ws_event_s
{
	void*					    data;
	ws_int32_t							fd;
	ws_int32_t							type;
	ws_int32_t							active;

	ws_int32_t							timer_set;
	ws_int32_t							timedout;
	ws_rbtree_node_t			timer;

	ws_event_handle_ptr			handle;

};

enum ws_event_type_s
{
	YMZ_EVENT_TYPE_ACCEPT     = 0x01,
	YMZ_EVENT_TYPE_READ       = 0x02,
	YMZ_EVENT_TYPE_WRITE      = 0x04,
	YMZ_EVENT_TYPE_ET         = 0x08,
	YMZ_EVENT_TYPE_ERROR      = 0x10,

};

ws_int32_t ws_event_process_init();
ws_int32_t ws_event_close();

void ws_event_process_and_expire();


ws_int32_t ws_event_add( ws_event_t* ev, ws_int32_t type );
ws_int32_t ws_event_del( ws_event_t* ev, ws_int32_t flags );
ws_int32_t ws_event_add_conn( void* conn );
ws_int32_t ws_event_del_conn( void* conn );
//ws_int32_t ws_event_mod( ws_event_t* ev, ws_int32_t type );

ws_int32_t ws_event_accept( ws_event_t* ev );
ws_int32_t ws_event_accept_udp( ws_event_t* ev );




#endif
