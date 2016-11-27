#ifndef __TOOL_H__
#define __TOOL_H__

#include <ws_types.h>

#ifndef _SAFE_RELEASE
#define _SAFE_RELEASE( ptr ) \
	if( ptr ) \
	{ \
	   delete ptr; \
	   ptr = 0; \
	} \

#endif

const ws_char_t* ws_str_find( const ws_char_t* str, char sb );
const ws_char_t* ws_str_find_not( const ws_char_t* str, char sb );

const ws_char_t* ws_str_find_not_len( const ws_char_t* str, char sb, ws_int32_t len );
const ws_char_t* ws_str_find_len( const ws_char_t* str, char sb, ws_int32_t len );

const ws_char_t* ws_str_find_not_last( const ws_char_t* str, char sb, const ws_char_t* last );
const ws_char_t* ws_str_find_last( const ws_char_t* str, char sb, const ws_char_t* last );

ws_int32_t ws_check_uri( const ws_char_t* uri );







#endif //__TOOL_H__
