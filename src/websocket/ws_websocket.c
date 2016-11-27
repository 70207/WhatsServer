#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_http.h>
#include <ws_websocket.h>
#include <ws_conn_send.h>
#include <ws_conn_websocket.h>
#include <ws_time.h>
#include <ws_log.h>

ws_int32_t              ws_websocket_handle(ws_conn_t  *c);

ws_int32_t              ws_websocket_read(ws_event_t    *ev);
ws_websocket_t*         ws_websocket_alloc(ws_conn_t* c);
ws_int32_t              ws_websocket_do(ws_websocket_t* wb);
ws_int32_t              ws_websocket_event_empty_send(ws_event_t* ev);

ws_int32_t              ws_websocket_block_read(ws_conn_t* c);
ws_int32_t              ws_websocket_read_cycle(ws_event_t* ev);


ws_int32_t          ws_websocket_conn_init(ws_conn_t *conn)
{
    ws_event_t                  *rev, *wev;

    rev = conn->rev;
    wev = conn->wev;

    ws_conn_nodelay(conn->s);
    rev->handle = ws_websocket_read_cycle;
    wev->handle = ws_websocket_event_empty_send;

    conn->data = 0;
    conn->type = YMZ_CONN_TYPE_WEBSOCKET;

    return ws_websocket_read_cycle(rev);
}







ws_int32_t ws_websocket_block_read(ws_conn_t* c)
{
    ws_event_del(c->rev, 0);
    return YMZ_CONN_OK;
}

#define YMZ_NR_OK	   0
#define YMZ_NR_AGAIN    1
#define YMZ_N

ws_int32_t ws_websocket_read_cycle(ws_event_t* ev)
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

    while ((ws_websocket_read(ev) == YMZ_NR_AGAIN) && times) {
        times--;
        continue;
    }

    return YMZ_OK;
}

ws_int32_t ws_websocket_read(ws_event_t    *ev)
{
    ws_websocket_t*							wb;
    ws_conn_t*							c;
    ws_mem_pool_t*						pool;
    ws_mem_buf_t*						buf;
    ws_clean_up_t*						cln;
    ws_int32_t									size;
    ws_int32_t									rtn;
    ws_int32_t									flag;
    ws_char_t*						        pos;
    ws_event_t*							wev;
    ws_int32_t									min;
    ws_int32_t									handled;


    c = ev->data;
    wev = c->wev;
    pool = c->pool;

    ws_log_access_debug("ws_websocket_read in, port:%d", c->port);


    if (ev->timedout || c->close) {
        ws_log_info("connection time out index:%d", c->index);
        ws_conn_close(c);
        return YMZ_NR_OK;
    }

    rtn = ws_conn_websocket_check_recv(c);
    if (!rtn) {
        rtn = ws_conn_websocket_recv(c);
        switch (rtn)
        {
        case YMZ_CONN_RECV_ERROR:
            ws_log_error("conn read fail, %llx\n", c);
            break;
        case YMZ_CONN_CLIENT_CLOSE:
            c->pending_state = 0;
            ws_conn_close(c);
            return YMZ_NR_OK;
        }
    }

    if (c->recv_mark) {
        rtn = ws_conn_websocket_check_recv(c);
        if (!rtn) {
            rtn = ws_conn_websocket_recv(c);
            switch (rtn)
            {
            case YMZ_CONN_RECV_ERROR:
                ws_log_error("conn read fail, %llx\n", c);
                break;
            case YMZ_CONN_CLIENT_CLOSE:
                c->pending_state = 0;
                ws_conn_close(c);
                return YMZ_NR_OK;
            }
        }
    }




    //-------------------------end of read--------------------//

    if (!c->data) {
        ws_websocket_alloc(c);
        wb = c->data;
    }
    else {
        wb = c->data;
    }

    handled = 0;
    min = c->read_buffer_size - 512 - c->recv_pos;
    while (!ws_conn_websocket_check_handle(c, &pos, &size)) {
        handled++;
        wb->net.read_buffer = pos;
        wb->net.read_buffer_size = size;
        wb->net.reh = pos;
        ws_websocket_do(wb);
    }


    if (c->send_wpos > c->send_pos) {
        wev->handle = ws_event_send;
        ws_event_send(wev);
    }



    return YMZ_NR_OK;

}


ws_websocket_t* ws_websocket_alloc(ws_conn_t* c)
{
    ws_mem_pool_t*			pool;
    ws_mem_buf_t*			buf;
    ws_websocket_t*				wb;
   // ws_busi_t*				busi;
    ws_int32_t						size;

    pool = c->pool;

    size = sizeof(ws_websocket_t);
    buf = ws_mem_buf_alloc(pool, size);
    wb = buf->data;
    memset(wb, 0, size);


   /* size = sizeof(ws_busi_t);
    buf = ws_mem_buf_alloc(pool, size);
    busi = buf->data;
    memset(busi, 0, size);



    busi->conn = c;
    busi->pool = pool;
    wb->net.pool = pool;
    wb->net.conn = c;
    busi->data = wb;
    wb->net.data = busi;*/
    wb->net.rsp = 0;
    c->data = wb;

    return wb;
}


ws_int32_t ws_websocket_do(ws_websocket_t* wb)
{
    ws_conn_t               *conn;
    ws_mem_pool_t           *pool;
    ws_mem_buf_t            *mem;
  //  ws_busi_t               *busi;
    ws_int32_t               size;
    ws_net_t                *net;
    ws_int32_t               rtn;

    net = &wb->net;
    conn = net->conn;
    pool = net->pool;

  /*  busi = (ws_busi_t*)net->data;

    busi->busi_state = YMZ_BUSI_TO_HANDLE;
    busi->type = YMZ_BUSI_TYPE_WEBSOCKET;
    busi->handle = ws_busi_request;


    rtn = ws_busi_request(busi);
*/
    ws_conn_check_send_buf(conn, conn->send_buf);

    return 0;
}

ws_int32_t ws_websocket_send(ws_websocket_t* wb)
{
    ws_conn_t* c;
    c = wb->net.conn;
    return ws_event_send(c->wev);
}



ws_int32_t ws_websocket_event_empty_send(ws_event_t* ev)
{
    return YMZ_OK;
}

ws_int32_t ws_websocket_close(ws_websocket_t* wb)
{

    if (!wb) {
        return YMZ_ERROR;
    }

    return 0;
}