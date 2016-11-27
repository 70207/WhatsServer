#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <ws_types.h>
#include <ws_str.h>


const ws_char_t* ws_str_find( const ws_char_t* str, char sb )
{
	if( !str )
		return 0;
	while( *str )
	{
		if( sb == *str )
			return str;
		++str;
	}

	return 0;
}

const ws_char_t* ws_str_find_not( const ws_char_t* str, char sb )
{
	if( !str )
		return 0;
	while( *str )
	{
		if( sb != *str )
			return str;
		++str;
	}

	return 0;
}

const ws_char_t* ws_str_find_not_len( const ws_char_t* str, char sb, ws_int32_t len )
{
	ws_int32_t cur;

	if( !str )
		return 0;
	for( cur=0; cur < len; ++cur )
	{
		if( sb != *( str + cur ) )
			return str + cur;
	}

	return 0;
}

const ws_char_t* ws_str_find_len( const ws_char_t* str, char sb, ws_int32_t len )
{
	ws_int32_t cur;

	if( !str )
		return 0;
	for( cur=0; cur < len; ++cur )
	{
		if( sb == *( str + cur ) )
			return str + cur;
	}

	return 0;
}

const ws_char_t* ws_str_find_not_last( const ws_char_t* str, char sb, const ws_char_t* last )
{
	if( !str )
		return 0;
	while( str != last )
	{
		if( sb != *str )
			return str;
		++str;
	}

	return 0;
}

const ws_char_t* ws_str_find_last( const ws_char_t* str, char sb, const ws_char_t* last )
{
	if( !str )
		return 0;
	while( str != last )
	{
		if( sb == *str )
			return str;
		++str;
	}

	return 0;
}


ws_int32_t ws_check_uri( const ws_char_t* uri )
{
	char first;

	if( !uri )
	{
		return -1;
	}

	first = uri[0] | 32;
	
	if( first < 97 || first > 122 )
	{
		return -1;
	}

	return 0;
}

