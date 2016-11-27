#ifndef _WS_LOG_H__
#define _WS_LOG_H__

#include <ws_types.h>

ws_int32_t ws_log_set_path(const ws_char_t *path);

#define LOGD ws_log_print_debug
#define LOGI ws_log_print_info
#define LOGW ws_log_print_warn
#define LOGE ws_log_print_error


#if defined(__linux__) || defined(__APPLE__) 

#define ws_log_access_debug( format, args... )				ws_log_access_real_debug( format, ##args )
#define ws_log_access_info( format, args... )				ws_log_access_real_info( format, ##args )
#define ws_log_access_warn( format, args... )				ws_log_access_real_warn( format, ##args )
#define ws_log_access_error( format, args... )				ws_log_access_real_error( format, ##args )

#define ws_log_debug( format, args... )					    ws_log_real_debug( format, ##args )
#define ws_log_info( format, args... )						ws_log_real_info( format, ##args )
#define ws_log_warn( format, args... )						ws_log_real_warn( format, ##args )
#define ws_log_error( format, args... )					    ws_log_real_error( format, ##args )

#define ws_log_sys_debug( format, args... )				    ws_log_sys_real_debug( format, ##args )
#define ws_log_sys_info( format, args... )					ws_log_sys_real_info( format, ##args )
#define ws_log_sys_warn( format, args... )					ws_log_sys_real_warn( format, ##args )
#define ws_log_sys_error( format, args... )				    ws_log_sys_real_error( format, ##args )


#define ws_log_print_debug( format, args... )			    ws_log_print_real_debug( format, ##args )
#define ws_log_print_info( format, args... )				ws_log_print_real_info( format, ##args )
#define ws_log_print_warn( format, args... )				ws_log_print_real_warn( format, ##args )
#define ws_log_print_error( format, args... )				ws_log_print_real_error( format, ##args )



#elif _WIN32

#define ws_log_access_debug( format, ... )				ws_log_access_real_debug( format, __VA_ARGS__ )
#define ws_log_access_info( format, ... )				ws_log_access_real_info( format, __VA_ARGS__ )
#define ws_log_access_warn( format, ... )				ws_log_access_real_warn( format, __VA_ARGS__ )
#define ws_log_access_error( format, ... )				ws_log_access_real_error( format, __VA_ARGS__ )

#define ws_log_debug( format, ... )					    ws_log_real_debug( format, __VA_ARGS__ )
#define ws_log_info( format, ... )						ws_log_real_info( format, __VA_ARGS__ )
#define ws_log_warn( format, ... )						ws_log_real_warn( format, __VA_ARGS__ )
#define ws_log_error( format, ... )					    ws_log_real_error( format, __VA_ARGS__ )

#define ws_log_sys_debug( format, ... )				    ws_log_sys_real_debug( format, __VA_ARGS__ )
#define ws_log_sys_info( format, ... )					ws_log_sys_real_info( format, __VA_ARGS__ )
#define ws_log_sys_warn( format, ... )					ws_log_sys_real_warn( format, __VA_ARGS__ )
#define ws_log_sys_error( format, ... )				    ws_log_sys_real_error( format, __VA_ARGS__ )


#define ws_log_print_debug( format, ... )			    ws_log_print_real_debug( format, __VA_ARGS__ )
#define ws_log_print_info( format, ... )				ws_log_print_real_info( format, __VA_ARGS__ )
#define ws_log_print_warn( format, ... )				ws_log_print_real_warn( format, __VA_ARGS__ )
#define ws_log_print_error( format, ... )				ws_log_print_real_error( format, __VA_ARGS__ )


#endif


ws_int32_t ws_log_real_debug(const ws_char_t* format, ...);
ws_int32_t ws_log_real_info(const ws_char_t* format, ...);
ws_int32_t ws_log_real_warn(const ws_char_t* format, ...);
ws_int32_t ws_log_real_error(const ws_char_t* format, ...);

ws_int32_t ws_log_print_real_debug(const ws_char_t* format, ...);
ws_int32_t ws_log_print_real_info(const ws_char_t* format, ...);
ws_int32_t ws_log_print_real_warn(const ws_char_t* format, ...);
ws_int32_t ws_log_print_real_error(const ws_char_t* format, ...);


ws_int32_t ws_log_access_real_debug(const ws_char_t* format, ...);
ws_int32_t ws_log_access_real_info(const ws_char_t* format, ...);
ws_int32_t ws_log_access_real_warn(const ws_char_t* format, ...);
ws_int32_t ws_log_access_real_error(const ws_char_t* format, ...);

ws_int32_t ws_log_sys_real_debug(const ws_char_t* format, ...);
ws_int32_t ws_log_sys_real_info(const ws_char_t* format, ...);
ws_int32_t ws_log_sys_real_warn(const ws_char_t* format, ...);
ws_int32_t ws_log_sys_real_error(const ws_char_t* format, ...);


#endif







