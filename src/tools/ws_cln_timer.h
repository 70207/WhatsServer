#ifndef _WS_CLN_TIMER_H__
#define _WS_CLN_TIMER_H__

#include <ws_rbtree.h>

typedef ws_int32_t (*ws_clean_up)( long data );
typedef ws_int32_t(*ws_clean_up2)(long data1, long data2);
typedef struct ws_clean_up_s ws_clean_up_t;
struct ws_clean_up_s
{
	ws_clean_up				clean_up;
	ws_clean_up				expired;
    ws_clean_up2            clean_up2;
	long					data;
	long					data2;
	long					data3;
	ws_int32_t						timer_set;
	ws_int32_t						timedout;
	ws_rbtree_node_t		timer;


	ws_clean_up_t*			prev;
	ws_clean_up_t*			next;
};


ws_int32_t ws_cln_timer_init();
ws_int32_t ws_cln_timer_add( ws_clean_up_t* cln,  ws_msec_t timer );
ws_int32_t ws_cln_timer_del( ws_clean_up_t* cln );
ws_int32_t ws_cln_timer_get();
void ws_cln_timer_expire();

extern ws_clean_up_t*		g_clean_free;



#endif _WS_CLN_TIMER_H__
