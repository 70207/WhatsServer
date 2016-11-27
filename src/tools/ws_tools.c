#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <ws_tools.h>
#include <ws_time.h>



static ws_int32_t hex_num[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8,
9,  0,  0,  0,  0,  0,  0, 0,
10, 11, 12, 13, 14, 15, 0, 0, 
0,  0,  0,  0,  0,  0,  0, 0, 
0,  0,  0,  0,  0,  0,  0, 0,
0,  0,  0,  0,  0,  0,  0, 0,
10, 11, 12, 13, 14, 15  };


ws_int32_t ws_hextostr( ws_uchar_t* src, ws_int32_t len, ws_uchar_t* dst )
{
	ws_int32_t hf, hs;
	ws_int32_t i;
	ws_int32_t half;

	half = len / 2;
	for( i = 0; i < half; ++i ){
		hf = src[ 2 * i ];
		hs = src[ 2 * i + 1 ];

		hf -= 0x30;
		hs -= 0x30;

		hf = hex_num[ hf ];
		hs = hex_num[ hs ];
		
		hf *= 0x10;
		dst[ i ] = hf + hs;
	}

	return 0;
}

static ws_int32_t hex_str[] = { '0', '1', '2', '3', '4', '5', '6','7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

ws_int32_t ws_strtohex( ws_uchar_t* src, ws_int32_t len, ws_uchar_t* dst )
{
	ws_int32_t hf, hs;
	ws_int32_t i;
	ws_int32_t half;

	for( i = 0; i< len; ++i ){
		hf = src[ i ] / 16;
		hs = src[ i ] % 16;

		dst[ 2 * i ] = hex_str[ hf ];
		dst[ 2 * i + 1 ] = hex_str[ hs ];
	}

	return 0;

}

ws_int32_t ws_random_16()
{
    return 0;
}

ws_int32_t ws_random_32()
{
    return 0;
}
ws_int32_t ws_random_num(ws_int32_t min, ws_int32_t max)
{

	srand(time(0));
	return rand() % (max - min + 1) + min;
}

ws_int32_t ws_random_data(ws_uchar_t* src, ws_int32_t size)
{
	ws_int32_t				r;
	ws_int32_t				i;
	if (!src){
		return 0;
	}

	srand(ws_current_sec);
	for (i = 0; i < size; ++i){
		r = rand() % 16;
		src[i] = (char)(r & 0x0f);
	}

	return src;

}
ws_int32_t ws_random_str(ws_char_t* const src, ws_int32_t size, ws_int32_t type )
{
	ws_int32_t i;
	ws_int32_t len;
	if (!src) {
		return 0;
	}
	const ws_char_t *strstr;

	srand(time(0));
	if (type == 1) {
		strstr = "abcdefghijklmnopqrstuvwxyz";
	}
	else if (type == 2) {
		strstr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}
	else if (type == 3) {
		strstr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}
	else if (type == 4) {
		strstr = "0123456789";
	}
	else {
		strstr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	}

	len = strlen(strstr);
	for (i = 0; i < size; ++i) {
		src[i] = strstr[rand() % len];
	}
	return 0;
}


