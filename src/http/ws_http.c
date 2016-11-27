#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <ws_smap.h>

#include <ws_log.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_http.h>
#include <ws_conn_http.h>
#include <ws_str.h>
#include <ws_conn_send.h>
#include <ws_websocket.h>
#include <ws_mem.h>



ws_int32_t ws_http_handle(ws_conn_t  *c);


ws_int32_t ws_http_net_module_init()
{

    return YMZ_CYCLE_OK;

}

ws_int32_t ws_http_handle(ws_conn_t  *c)
{
    ws_event_t* rev, *wev;

    rev = c->rev;
    wev = c->wev;

    c->type = YMZ_CONN_TYPE_HTTP;


    ws_conn_nodelay(c->s);
    rev->handle = ws_http_event_read_handle;
    wev->handle = ws_http_event_write_handle;


    rev->handle(rev);

    return YMZ_CONN_OK;
}

ws_int32_t  ws_http_event_read_handle(ws_event_t *ev)
{
    ws_conn_t							       *c;
    ws_int32_t									times;
    ws_http_t                                  *http;
    ws_event_t                                 *wev;


    times = 1;
    c = ev->data;


    if (ev->timedout) {
        ws_log_print_debug("http connection time out");
        ws_conn_close(c);
        return YMZ_OK;
    }

    ws_event_timer_add(ev, c->life_time);

    if (c->data) {
        http = c->data;
    }
    else {
        http = ws_http_alloc(c);
        c->data = http;
    }

    ws_http_read(c);

    if (YMZ_OK != ws_http_request_check(c)) {
        return YMZ_AGAIN;
    }

    ws_http_request_parse(http);
    ws_http_do(http);

    wev = c->wev;

    if (c->send_wpos > c->send_pos) {
        wev->handle = ws_event_send;
        ws_event_send(wev);
    }

    ws_websocket_conn_init(c);

    return YMZ_OK;
}
ws_int32_t  ws_http_event_write_handle(ws_event_t *ev)
{
    ws_conn_t*							c;
    ws_int32_t									times;


    times = 1;
    c = ev->data;


    if (ev->timedout) {
        ws_log_print_debug("http connection time out");
        ws_conn_close(c);
        return YMZ_OK;
    }

    ws_event_timer_add(ev, c->life_time);

    return YMZ_OK;
}


ws_int32_t ws_http_do(ws_http_t* http)
{
    //ws_conn_t* conn;
    //ws_mem_pool_t* pool;
    //ws_mem_buf_t* mem;
    //ws_busi_t* busi;
    //ws_int32_t size;


    //conn = http->conn;
    //pool = http->pool;

    //busi = (ws_busi_t*)http->data;

    //busi->busi_state = YMZ_BUSI_TO_HANDLE;
    //busi->type = YMZ_BUSI_TYPE_HTTP;
    //busi->handle = ws_busi_request;

    //
    //return ws_busi_request(busi);

    return YMZ_OK;
}


ws_http_t* ws_http_alloc(ws_conn_t* conn)
{
    ws_mem_pool_t* pool;
    ws_mem_buf_t* mem;
    ws_http_t* http;
    ws_http_request_t* request;
    ws_http_header_t* header;
    ws_http_cookie_t* cookie;
    ws_http_cookie_t* mod_cookie;
    ws_smap_t* smap;
    ws_int32_t size;

    pool = conn->pool;

    size = sizeof(ws_http_t);
    mem = ws_mem_buf_alloc(pool, size);
    http = (ws_http_t*)mem->data;
    memset(http, 0, size);


    //size = sizeof(ws_busi_t);
    //mem = ws_mem_buf_alloc(pool, size);
    //busi = (ws_busi_t*)mem->data;
    //memset(busi, 0, size);


    //busi->conn = conn;
    //http->conn = conn;
    //busi->pool = pool;
    //http->pool = pool;
    //busi->http = http;
    //http->data = busi;
    conn->data = http;

    return http;
}

ws_int32_t ws_http_free(ws_http_t* http)
{
    ws_mem_pool_t* pool;
    ws_mem_buf_t* mem;

    pool = http->pool;
    ws_mem_pool_free(pool);

    return 0;
}



ws_int32_t ws_http_read(ws_conn_t* conn)
{
    ws_int32_t rtn;
    ws_int32_t i;
    ws_http_t* http;
    ws_mem_buf_t* mem;
    ws_int32_t size;
    ws_int32_t flag;

    printf("FUNC:%s\n", __FUNCTION__);

    rtn = ws_conn_http_recv(conn);
    switch (rtn)
    {
    case YMZ_CONN_RECV_ERROR:
        printf("conn read fail, %llx\n", conn);
        return rtn;
    case YMZ_CONN_CLIENT_CLOSE:
        conn->pending_state = 1;
        return rtn;
    case YMZ_CONN_RECV_OK:
        conn->pending_state = 0;
        break;
    }

    printf("end of read\n");

    
    


    return YMZ_CONN_OK;
}

ws_int32_t  ws_http_request_check(ws_conn_t *conn)
{
    ws_int32_t                                  state;
    ws_char_t                                  *data;
    ws_int32_t                                  pos;
    ws_mem_buf_t                               *buf;

#define YMZ_HTTP_REQUEST_CHECK_NO                0
#define YMZ_HTTP_REQUEST_CHECK_R                 1
#define YMZ_HTTP_REQUEST_CHECK_N                 2
#define YMZ_HTTP_REQUEST_CHECK_R2                3
#define YMZ_HTTP_REQUEST_CHECK_N2                4

    state = YMZ_HTTP_REQUEST_CHECK_N2;

    buf = conn->recv_buf;
    data = buf->data + conn->recv_pos;

    while (data > buf->data) {
        if (*data == '\n') {
            break;
        }
        data--;
    }

    if (*data != '\n') {
        return YMZ_AGAIN;
    }

    data -= 3;
    if (data[0] != '\r' || data[1] != '\n' || data[2] != '\r' || data[3] != '\n') {
        return YMZ_AGAIN;
    }

    data -= 1;
    while (data > buf->data) {
        if (*data == '\n') {
            break;
        }
        data--;
    }

    if (*data != '\n') {
        return YMZ_AGAIN;
    }

    data -= 3;
    if (data[0] != '\r' || data[1] != '\n' || data[2] != '\r' || data[3] != '\n') {
        return YMZ_AGAIN;
    }

    return YMZ_OK;
}

ws_int32_t ws_http_parse_header(ws_http_t* http);
ws_int32_t ws_http_parse_request(ws_http_t* http);
ws_int32_t ws_http_parse_cookie(ws_http_t* http);
ws_int32_t ws_http_parse_param(ws_http_t* http, const ws_char_t* param, const ws_char_t* last);

ws_int32_t  ws_http_request_parse(ws_http_t* http)
{
    ws_log_debug("FUNC:%s\n", __FUNCTION__);

    ws_http_parse_request(http);
    ws_http_parse_header(http);
    ws_http_parse_cookie(http);

    return YMZ_OK;
}

ws_int32_t ws_http_parse_request( ws_http_t* http )
{
	ws_http_request_t                       *request;
	ws_mem_pool_t                           *pool;
	ws_mem_buf_t                            *mem;
	ws_conn_t                               *conn;
	const ws_char_t                         *r_buf;
	ws_int32_t                               r_len;
	const ws_char_t                         *cur;
	const ws_char_t                         *nc;
	const ws_char_t                         *nc2;
	const ws_char_t                         *last;
	ws_char_t                                buffer[ 32 ];
	ws_int32_t                               i;
	ws_int32_t                               size;

	conn = http->conn;
	pool = http->pool;
	request = &http->request;
    mem = conn->recv_buf;
    r_buf = mem->data;
    r_len = conn->recv_pos;
    last = r_buf + r_len;

	
 	ws_log_debug( "FUNC:%s\n", __FUNCTION__ );	
	cur = ws_str_find_not_last( r_buf, ' ',  last );
	if( !cur )
	{
		return YMZ_HTTP_PARSE_REQUEST_FAIL;
	}

	
	
	if( ((*cur )| 32) == 'g' )
	{
		http->type = YMZ_HTTP_GET;
	}
	else if(((*cur) | 32) == 'p')
	{
		http->type = YMZ_HTTP_POST;
	}
    else {
        http->type = YMZ_HTTP_NONE;
        ws_log_error("error http type:%d", *cur);
    }
	
	
	nc = ws_str_find_last( cur, ' ', last );  
	if( !nc )
	{
		return YMZ_HTTP_PARSE_REQUEST_FAIL;
	}

	cur = nc + 2;
	request->request = cur;
	nc = ws_str_find_last( cur, ' ', last );
	nc2 = ws_str_find_last( cur, '?', nc );
	if( nc2 )
	{
		request->has_uri = 1;
		size = nc2 - cur;
	}
	else
	{
		size = nc - cur;
	}

	mem = ws_mem_buf_alloc( pool, size + 1 );
	request->uri = mem->data;

	memcpy( request->uri, cur, size );
	request->uri[ size ] = 0;



	if( nc2 )
	{
		++nc2;
		ws_http_parse_param( http, nc2, nc );
	}


	return 0;
}

ws_int32_t ws_http_parse_header( ws_http_t  *http )
{
	ws_http_header_t               *header;
	ws_mem_pool_t                  *pool;
	ws_mem_buf_t                   *mem;
	ws_conn_t                      *conn;
	const ws_char_t                *r_buf;
	ws_int32_t                      r_len;
	ws_char_t                      *cur;
    ws_char_t                      *nc, *nc2, *prv, *pnv, *pfv;
	ws_char_t                      *last;
	ws_char_t                       buffer[ 64 ];
	ws_int32_t                      i;
	ws_int32_t                      size;

    conn = http->conn;
    pool = http->pool;
    mem = conn->recv_buf;
    r_buf = mem->data;
    r_len = conn->recv_pos;
    last = r_buf + r_len;
	cur = r_buf;
	
 	ws_log_debug( "FUNC:%s\n", __FUNCTION__ );	

    cur = strstr(r_buf, "\r\n");

    cur += 4;

    while (1)
    {
        cur = ws_str_find_not(cur, ' ');
        nc = ws_str_find(cur, ':');
        nc2 = ws_str_find(cur, '\r');
        if (nc)
        {
            *nc = 0;
            if (!nc2)
            {
                pnv = ws_str_find(nc + 1, '\n');
                pfv = ws_str_find(nc + 1, ' ');
             

                if (pfv && pnv > pfv)
                {
                    pnv = pfv;
                }

                *pnv = 0;
            }
            else {
                *nc2 = 0;
            }


            ws_smap_insert(&http->header.smap, cur, nc + 1);
        }

        if (!nc || !nc2) {
            break;
        }

       
        cur = nc2 + 4;
    }


	return YMZ_OK;

}



ws_int32_t ws_http_parse_cookie( ws_http_t* http )
{
    ws_http_header_t               *header;
    ws_mem_pool_t                  *pool;
    ws_mem_buf_t                   *mem;
    ws_conn_t                      *conn;
    const ws_char_t                *r_buf;
    ws_int32_t                      r_len;
    ws_char_t                       *cur;
    ws_char_t                       *nc, *nc2, *prv, *pnv, *pfv;
    ws_char_t                       *last;
    ws_char_t                       buffer[64];
    ws_int32_t                      i, len1, len2;
    ws_int32_t                      size;
 

    conn = http->conn;
    pool = http->pool;
    mem = conn->recv_buf;
    r_buf = mem->data;
    r_len = conn->recv_pos;
    last = r_buf + r_len;
    cur = r_buf;


	cur = strstr( r_buf, "Cookie" );

 	ws_log_debug( "FUNC:%s\n", __FUNCTION__ );	

    cur += 7;
   
  
    while (1)
    {
        cur = ws_str_find_not(cur, ' ');
        nc = ws_str_find(cur, '=');
        nc2 = ws_str_find(cur, ';');
        if (nc)
        {
            *nc = 0;
            if (!nc2)
            {
                prv = ws_str_find(nc + 1, '\r');
                pnv = ws_str_find(nc + 1, '\n');
                pfv = ws_str_find(nc + 1, ' ');
                if (pnv < prv)
                {
                    prv = pnv;
                }

                if (pfv && prv > pfv)
                {
                    prv = pfv;
                }

                *prv = 0;
            }
            else {
                *nc2 = 0;
            }

            
            ws_smap_insert(&http->cookie.smap, cur, nc + 1);
        }

        if (!nc || !nc2) {
            break;
        }
        cur = nc2 + 1;
    }

	return YMZ_OK;
	
}


ws_int32_t ws_http_parse_param( ws_http_t* http, const ws_char_t* param, const ws_char_t* last )
{
	ws_http_request_t* request;
	ws_mem_pool_t* pool;
	ws_mem_buf_t* mem;
	ws_smap_t* smap;
	ws_int32_t len;
	ws_char_t* nc;
	ws_char_t* nc2;
	const ws_char_t* name;
	const ws_char_t* value;
	

	if( !http || !param || !last )
	{
		return -1;
	}

	pool = http->pool;
	request = &http->request;
    smap = &request->smap;
	
 	ws_log_debug( "FUNC:%s\n", __FUNCTION__ );	
	

	nc2 = param;
	while( nc2 < last )
	{
		nc = ws_str_find_last( param, '=', last );
		name = param;
		nc2 = ws_str_find_last( nc, '&', last );
		if( !nc2 )
		{
			nc2 = last;
		}

		*nc = 0;
		*nc2 = 0;
		value = ++nc;

		ws_smap_insert( smap, name, value );

	}


	return YMZ_OK;
}




ws_int32_t ws_http_worker_connect(ws_http_t   *http)
{
    ws_http_request_t                   *req;
    ws_int8_t                            buffer[1024];

    ws_mem_buf_t                        *buf;
    ws_mem_pool_t                       *pool;
    ws_int32_t                           size;
    ws_char_t                           *accept;

    if (!http) {
        ws_log_access_warn("wront http connect!");
        return YMZ_ERROR;
    }


    if (!http->websocket_req.sec_websocket_key) {
        ws_log_access_warn("wront http connect, there is no websocket key!");
        return YMZ_ERROR;
    }
/*


    pool = http->pool;

    size = SHA_DIGEST_LENGTH * 4;

    buf = ws_mem_buf_alloc(pool, size);
    if (!buf) {
        ws_log_access_warn("http connect but not enough mem!");
        return YMZ_ERROR;
    }

    size = sprintf(buffer, "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", http->websocket_req.sec_websocket_key);

    SHA1(buffer, size, buf->data);

    http->websocket_rsp.sec_websocket_accept = buf->data;*/

    return YMZ_OK;
}
