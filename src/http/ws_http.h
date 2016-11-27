#ifndef _WS_HTTP_H__
#define _WS_HTTP_H__


#include <ws_types.h>
#include <ws_event.h>
#include <ws_smap.h>
#include <ws_mem.h>
#include <ws_conn.h>






typedef struct ws_http_header_s ws_http_header_t;
typedef struct ws_http_request_s ws_http_request_t;
typedef struct ws_http_cookie_s ws_http_cookie_t;
typedef struct ws_http_s ws_http_t;
typedef enum ws_http_type_s ws_http_type_t;
typedef struct ws_http_websocket_req_s      ws_http_websocket_req_t;
typedef struct ws_http_websocket_rsp_s      ws_http_websocket_rsp_t;

enum ws_http_type_s
{
    YMZ_HTTP_NONE,
    YMZ_HTTP_GET,
    YMZ_HTTP_POST,
};


struct ws_http_websocket_req_s
{
    ws_char_t*              upgrade;
    ws_char_t*              connection;
    ws_char_t*              host;
    ws_char_t*              origin;
    ws_char_t*              sec_websocket_key;
    ws_char_t*              sec_websocket_version;
    ws_char_t*              sec_websocket_extensions;
};


struct ws_http_websocket_rsp_s
{
    ws_char_t*              upgrade;
    ws_char_t*              connection;
    ws_char_t*              sec_websocket_accept;
    ws_char_t*              websocket_origin;
    ws_char_t*              websocket_location;
};

struct ws_http_header_s
{
    const ws_char_t* header;
    ws_char_t* host;
    ws_char_t* connection;
    ws_char_t* cache_control;
    ws_char_t* user_agent;
    ws_char_t* accept;
    ws_char_t* accept_encoding;
    ws_char_t* accept_language;
    ws_char_t* accept_charset;

    ws_char_t* server;
    ws_char_t* date;
    ws_char_t* content_type;
    ws_char_t* transfer_encoding;
    ws_char_t* p3p;
    ws_char_t* expires;
    ws_char_t* content_encoding;

    ws_smap_t    smap;

};


struct ws_http_request_s
{
    const ws_char_t* request;
    ws_char_t* uri;
    ws_int32_t has_uri;
    ws_smap_t  smap;
};

struct ws_http_cookie_s
{
    const ws_char_t* cookie;
    ws_smap_t smap;
};


struct ws_http_s
{
    ws_mem_pool_t*                  pool;
    void*                           data;
    ws_http_header_t                header;
    ws_http_request_t               request;
    ws_http_cookie_t                cookie;
    ws_http_cookie_t                mod_cookie;

    ws_http_websocket_req_t         websocket_req;
    ws_http_websocket_rsp_t         websocket_rsp;

    ws_conn_t* conn;
    ws_int32_t type;
};


ws_int32_t ws_http_read(ws_conn_t* conn);
ws_int32_t ws_http_send_init_header(ws_http_t* http);
ws_int32_t ws_http_send(ws_conn_t* conn);

ws_http_t* ws_http_alloc(ws_conn_t* conn);
ws_int32_t ws_http_free(ws_http_t* http);
ws_int32_t ws_http_worker_connect(ws_http_t   *http);

ws_int32_t  ws_http_module_init();
ws_int32_t  ws_http_event_read_handle(ws_event_t *ev);
ws_int32_t  ws_http_event_write_handle(ws_event_t *ev);


ws_int32_t  ws_http_request_check(ws_conn_t *conn);

enum yumei_http_parse_type_s
{
    YMZ_HTTP_PARSE_OK = 0,
    YMZ_HTTP_PARSE_REQUEST_FAIL,
    YMZ_HTTP_PARSE_HEADER_FAIL,
    YMZ_HTTP_PARSE_COOKIE_FAIL,
};

ws_int32_t  ws_http_request_parse(ws_http_t* http);

#endif