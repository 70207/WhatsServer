#ifndef _WS_CONF_H__
#define _WS_CONF_H__

ws_int32_t ws_conf_module_init( ws_cycle_t* cycle );
ws_int32_t ws_conf_get( const ws_char_t* name, const ws_char_t** value, ws_int32_t* len );

extern ws_char_t* ws_conf_path;


#endif //_WS_CONF_H__
