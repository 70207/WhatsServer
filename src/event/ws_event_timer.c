#include <ws_std.h>
#include <ws_inet.h>

#include <ws_log.h>
#include <ws_mem.h>

#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_event.h>
#include <ws_time.h>
#include <ws_event_timer.h>

#define YMZ_EVENT_TIMER_LAZY_DIFF				300000

ws_thread_volatile ws_rbtree_t  ws_event_timer_rbtree;
static ws_rbtree_node_t          ws_event_timer_sentinel;

ws_int32_t ws_event_timer_init()
{
	ws_rbtree_init(&ws_event_timer_rbtree, &ws_event_timer_sentinel,
    ws_rbtree_insert_timer_value);

	return YMZ_EVENT_OK;
}

ws_int32_t ws_event_timer_add( ws_event_t* ev,  ws_msec_t timer )
{
	ws_msec_t				time;
	ws_int64_t				key;
	long long   				diff;

	time = ws_current_msec + timer;
	key = ws_time_times_get_static(time);

	if( ev->timer_set ){
		diff = key - ev->timer.key.v;
		if( ws_abs( diff ) < YMZ_EVENT_TIMER_LAZY_DIFF ){
			return 0;
		}

		ws_event_timer_del( ev );
	}

	key = ws_time_times_get(time);
	ev->timer.key.v = key;
    ev->timer.key.v2 = 0;
	ws_rbtree_insert( &ws_event_timer_rbtree, &ev->timer );

	ev->timer_set = 1;
	ev->timedout = 0;
	return 0;

}

ws_int32_t ws_event_timer_del( ws_event_t* ev )
{
	ws_rbtree_delete( &ws_event_timer_rbtree, &ev->timer );
	ev->timer_set = 0;
	return 0;

}

time_t ws_event_timer_get()
{
	return 0;
}

void ws_event_timer_expire()
{
	 ws_rbtree_node_t       *node, *root, *sentinel;
	 ws_event_t*			ev;
	 ws_int64_t				timer;

	 timer = ws_current_times_get();

     sentinel = ws_event_timer_rbtree.sentinel;

	  for ( ;; ) {
		   root = ws_event_timer_rbtree.root;
		   if (root == sentinel) {
				return;
			}

		    node = ws_rbtree_min(root, sentinel);

			if( ( long long )(node->key.v - timer) <= 0 )
			{
				ev = (ws_event_t *) ((char *) node - offsetof(ws_event_t, timer));
				ws_rbtree_delete( &ws_event_timer_rbtree, &ev->timer );
				ev->timedout = 1;
				ev->timer_set = 0;
				ev->handle( ev );
				continue;
			}

			break;

	  }
	return;
}

