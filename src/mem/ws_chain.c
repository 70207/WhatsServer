#include <ws_std.h>
#ifdef __linux__
#include <malloc.h>
#endif

#include <ws_types.h>
#include <ws_chain.h>

ws_int32_t ws_chain_free_part( ws_chain_t* chain );

ws_chain_t* ws_chain_insert( ws_chain_t* chain, ws_int32_t length )
{
	ws_chain_t* prev;
	ws_int32_t size;
	ws_uchar_t* buf;

	size = sizeof( ws_chain_t );
	buf = ( ws_chain_t* ) malloc( size + length );
	
	prev = buf;
	buf += size;

	prev->io_len = prev->io_pos = prev->first_set = 0;

	prev->buf = buf;
	prev->prev = 0;
	prev->length = length;

	if( chain )
	{
		chain->prev = prev;
		prev->next=  chain;
	}

	return prev;
	
}
ws_chain_t* ws_chain_append( ws_chain_t* chain, ws_int32_t length )
{
	ws_chain_t* next;
	ws_int32_t size;
	ws_uchar_t* buf;

	next = ws_chain_get();
	if( !next ){
		return 0;
	}

	if( !chain ){
		next->prev = next->next = 0;
		return next;
	}

	while( chain->next )
	{
		chain = chain->next;
	}

	chain->next = next;
	next->prev = chain;
	
	return next;

}

ws_chain_t* ws_chain_link( ws_chain_t* prev_chain, ws_chain_t* next_chain )
{
	ws_chain_t* chain;
	if( !prev_chain )
	{
		return next_chain;
	}

	if( !next_chain )
	{
		return prev_chain;
	}

	chain = prev_chain;
	while( chain->next )
	{
		chain = chain->next;
	}
	chain->next = next_chain;
	next_chain->prev = chain;

	
	return prev_chain;
}

ws_chain_t* ws_chain_get_next( ws_chain_t* chain )
{
	ws_chain_t* _chain;
	if( !chain )
	{
		return 0;
	}
	
	_chain = chain;
	_chain->first_set = 0;

	while( _chain )
	{
		if( _chain->first_set )
		{
			ws_chain_free_part( chain );
			chain = _chain;
			break;
		}

		if( _chain->io_pos < _chain->io_len )
		{
			break;
		}

		
		_chain = _chain->next;
	}

	chain->prev = 0;
	chain->first_set = 0;
	return chain;
}


ws_int32_t ws_chain_free_part( ws_chain_t* chain )
{
	ws_chain_t* next;
	if( !chain )
	{
		return -1;
	}

	while( chain )
	{
		if( chain->first_set )
		{
			break;
		}
		
		next = chain->next;
		ws_chain_put( chain );
		

		chain = next;
		
	}

	return 0;
}

ws_int32_t ws_chain_free( ws_chain_t* chain )
{
	ws_chain_t* next;
	if( !chain )
	{
		return -1;
	}

	while( chain )
	{
		next = chain->next;
		ws_chain_put( chain );
		
		chain = next;
	}

	return 0;
}

ws_int32_t ws_chain_free_item(  ws_chain_t* chain )
{
	ws_chain_t* next;
	if( !chain )
	{
		return -1;
	}

	free( chain );	

	return 0;
}

ws_int32_t ws_chain_clear_io( ws_chain_t* chain )
{
	if( !chain )
	{
		return -1;
	}

	while( chain )
	{
		if( chain->first_set )
		{
			break;
		}

		chain->io_pos = 0;
		chain = chain->next;
	}

	return 0;
}

ws_int32_t ws_chain_first_set( ws_chain_t* chain )
{
	if( !chain )
	{
		return -1;
	}

	chain->first_set = 1;

	return 0;
}

#define YMZ_CHAIN_MAX_SIZE    256
#define YMZ_CHAIN_MAX_NUM	  1048576

ws_chain_t*		ws_chain_f = 0;
ws_int32_t					ws_chain_n = 0;
ws_int32_t ws_chain_module_init()
{
	ws_int32_t					size;
	ws_char_t*				data;
	ws_chain_t*		chain;
	ws_int32_t					i;

	size = 256;

	data = malloc( YMZ_CHAIN_MAX_SIZE * YMZ_CHAIN_MAX_NUM );
	if( !data ){
		return YMZ_ERROR;
	}

	chain = data;
	ws_chain_f = chain;

	for( i = 1; i < YMZ_CHAIN_MAX_NUM; ++i ){
		chain = data;
		chain->length = YMZ_CHAIN_MAX_SIZE;
		chain->buf = data + sizeof( ws_chain_t );
		data += YMZ_CHAIN_MAX_SIZE;
		chain->next = data;
	}

	chain = data;
	chain->length = YMZ_CHAIN_MAX_SIZE;
	chain->next = 0;
	chain->buf = data + sizeof( ws_chain_t );

	ws_chain_n = YMZ_CHAIN_MAX_NUM;

	return YMZ_OK;
}

ws_chain_t* ws_chain_get()
{
	ws_chain_t* chain = 0;

	if( !ws_chain_free ){
		return 0;
	}

	chain = ws_chain_f;
	ws_chain_f = chain->next;

	chain->next = chain->prev = 0;
	chain->io_len = chain->io_pos = chain->first_set = 0;
	ws_chain_n--;
	return chain;
}

void ws_chain_put( ws_chain_t* chain )
{
	if( !chain ){
		return;
	}

	chain->next = ws_chain_f;
	ws_chain_f = chain;

	ws_chain_n++;
}