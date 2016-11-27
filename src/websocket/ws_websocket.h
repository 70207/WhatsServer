#ifndef _WS_WEBSOCKET_H__
#define _WS_WEBSOCKET_H__

#include <ws_types.h>
#include <ws_conn.h>
#include <ws_net.h>

typedef struct ws_websocket_header_s    ws_websocket_header_t;
typedef struct ws_websocket_s           ws_websocket_t;

struct ws_websocket_header_s
{
    ws_uint32_t                 fin : 1;
    ws_uint32_t                 rsv_1 : 1;
    ws_uint32_t                 rsv_2 : 1;
    ws_uint32_t                 rsv_3 : 1;

    ws_uint32_t                 opcode : 4;

    ws_uint32_t                 mask : 1;

    ws_uint32_t                 payload_len : 7;

    union {
        ws_uint32_t                    len_16;
        ws_uint64_t                    len_64; 
    }extend_payload_length;
    
};

struct ws_websocket_s
{
    ws_websocket_header_t               header;
    ws_char_t                           mask_key[4];

    ws_net_t                            net;
};

ws_int32_t  ws_websocket_conn_init(ws_conn_t *conn);

#endif