#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <ws_types.h>

ws_int32_t ws_hextostr( ws_uchar_t* src, ws_int32_t len, ws_uchar_t* dst );
ws_int32_t ws_strtohex( ws_uchar_t* src, ws_int32_t len, ws_uchar_t* dst );
ws_int32_t ws_random_16();
ws_int32_t ws_random_32();
ws_int32_t ws_random_num(ws_int32_t min,ws_int32_t max);

ws_int32_t ws_random_data(ws_uchar_t* src, ws_int32_t size);
ws_int32_t ws_random_str(ws_char_t* const src, ws_int32_t size, ws_int32_t type);
#endif //__TOOLS_H__
