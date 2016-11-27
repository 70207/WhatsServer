#ifndef _WS_LOG_CORE_H__
#define _WS_LOG_CORE_H__

#include <ws_types.h>

ws_int32_t ws_log_init();
ws_int32_t ws_log_set_mode(ws_int32_t mode);
ws_int32_t ws_log_set_level(ws_int32_t level);
ws_int32_t ws_log_set_access(const ws_char_t* path);
ws_int32_t ws_log_set_error(const ws_char_t* path);
ws_int32_t ws_log_set_report(const ws_char_t* path);
ws_int32_t ws_log_close();


#define YMZ_LOG_BUFFER_DEFAULT			2048

#endif