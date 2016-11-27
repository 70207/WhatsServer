#ifndef __YUMEI_CHAIN_H__
#define __YUMEI_CHAIN_H__

#include <ws_types.h>
typedef struct ws_chain_s ws_chain_t;

struct ws_chain_s
{
	ws_uchar_t* buf;
	ws_int32_t length;
	ws_int32_t io_len;
	ws_int32_t io_pos;
	ws_int32_t first_set;
	ws_chain_t* next;
	ws_chain_t* prev;

	long data;
	
};

ws_chain_t* ws_chain_insert( ws_chain_t* chain, ws_int32_t length );
ws_chain_t* ws_chain_append( ws_chain_t* chain, ws_int32_t length );

ws_chain_t* ws_chain_link( ws_chain_t* prev_chain, ws_chain_t* next_chain );
ws_chain_t* ws_chain_get_next( ws_chain_t* chain );

ws_chain_t* ws_chain_alloc();
ws_int32_t ws_chain_free( ws_chain_t* chain );
ws_int32_t ws_chain_free_item(  ws_chain_t* chain );
ws_int32_t ws_chain_clear_io( ws_chain_t* chain );
ws_int32_t ws_chain_first_set( ws_chain_t* chain );

ws_int32_t ws_chain_module_init();

ws_chain_t* ws_chain_get();
void ws_chain_put( ws_chain_t* chain );

extern ws_chain_t*		ws_chain_f;
extern ws_int32_t					ws_chain_n;

#endif //__YUMEI_CHAIN_H__
