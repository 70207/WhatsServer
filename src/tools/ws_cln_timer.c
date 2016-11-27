#include <ws_std.h>
#include <string.h>

#include <ws_log.h>
#include <ws_mem.h>


#include <ws_cln_timer.h>
#include <ws_time.h>


#define YMZ_CLN_TIMER_LAZY_DIFF				300

ws_clean_up_t*		g_clean_free;

ws_thread_volatile ws_rbtree_t  ws_cln_timer_rbtree;
static ws_rbtree_node_t          ws_cln_timer_sentinel;

ws_int32_t ws_cln_timer_init()
{
	ws_rbtree_init(&ws_cln_timer_rbtree, &ws_cln_timer_sentinel,
    ws_rbtree_insert_timer_value);

	//if( !ws_cln_timer_alloc() ){
	//	return YMZ_ERROR;
	//}

	return YMZ_OK;

}

ws_int32_t ws_cln_timer_add( ws_clean_up_t* cln,  ws_msec_t timer )
{
	ws_msec_t				key;
	long long   				diff;

	key =  ws_current_msec + timer;

	if( cln->timer_set ){
		diff = key - cln->timer.key.v;
		if( ws_abs( diff ) < YMZ_CLN_TIMER_LAZY_DIFF ){
			return 0;
		}

		ws_cln_timer_del( cln );
	}

	cln->timer.key.v = key;
    cln->timer.key.v2 = 0;

	ws_rbtree_insert( &ws_cln_timer_rbtree, &cln->timer );

	cln->timer_set = 1;
	cln->timedout = 0;
	return 0;

}

ws_int32_t ws_cln_timer_del( ws_clean_up_t* cln )
{
	ws_rbtree_delete( &ws_cln_timer_rbtree, &cln->timer );
	cln->timer_set = 0;
	return 0;
}

ws_int32_t ws_cln_timer_get()
{
	return 0;
}

void ws_cln_timer_expire()
{
	 ws_rbtree_node_t      *node, *root, *sentinel;
	 ws_clean_up_t*	     cln;

     sentinel = ws_cln_timer_rbtree.sentinel;

	  for ( ;; ) {
		   root = ws_cln_timer_rbtree.root;
		   if (root == sentinel) {
				return;
			}

		    node = ws_rbtree_min(root, sentinel);

			if( ( long )(node->key.v - ws_current_msec) <= 0 )
			{
				cln = (ws_clean_up_t *) ((char *) node - offsetof(ws_clean_up_t, timer));
				ws_rbtree_delete( &ws_cln_timer_rbtree, &cln->timer );
				cln->timedout = 1;
				cln->timer_set = 0;
				cln->expired( cln );
				continue;
			}

			break;

	  }
	return;

}

//
//ws_int32_t ws_cln_timer_alloc()
//{
//	ws_clean_up_t*			cln, *tp, *tp2;
//	ws_int32_t						num;
//	ws_int32_t						size;
//	ws_int32_t						i;
//
//	num = ws_cycle->free_client_n;
//
//	size = sizeof( ws_clean_up_t ) * num;
//
//	cln = malloc( size );
//	if( !cln ){
//		return	YMZ_ERROR;
//	}
//
//	memset( cln, 0, size );
//	
//	tp = cln;
//	for( i = 1; i < size; ++i ){
//		tp2 = tp + 1;
//		tp->next = tp2;
//		tp2->prev = tp;
//	}
//
//	
//
//	g_clean_free = cln;
//	return YMZ_OK;
//
//}