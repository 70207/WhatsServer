#include <signal.h>
#include <errno.h>

#include <ws_std.h>
#include <ws_inet.h>
#include <ws_mem.h>
#include <ws_chain.h>
#include <ws_event.h>
#include <ws_conn_http.h>
#include <ws_cycle.h>
#include <ws_log.h>
#include <ws_process.h>

ws_int32_t ws_conn_http_recv(ws_conn_t  *c)
{
    ws_int32_t                   size;
    ws_int32_t                   curr;
    ws_int32_t                   flag;
    ws_int32_t                   err;
    ws_int32_t                   all_size;
    ws_int32_t					  msize;
    ws_mem_pool_t		 *pool;
    ws_mem_buf_t		 *recv_buf;
    ws_char_t*				  pos;
    ws_int32_t 				  header;
    ws_int32_t					  rc;
    ws_int32_t					  seqid;


    ws_log_access_debug("ws_conn_http_recv in maxrecv:%d", c->max_recv);
    pool = c->pool;
    recv_buf = c->recv_buf;

    size = c->recv_pos;
    all_size = size + c->max_recv;
    curr = 0;

    msize = c->max_recv;
    rc = 0;
    while (size < all_size) {
        flag = recv(c->s, recv_buf->data + size, msize, 0);
        if (flag < 0) {
            err = errno;
            if (err == EAGAIN) {
                rc = EAGAIN;
                ws_log_info("recv http again, recv_pos:%d, last_finish:%d, mark:%d", c->recv_pos, c->last_finish, c->recv_mark);
                break;
            }
            else if (err == EINTR) {
                continue;
            }

            ws_log_warn("recv error");
            return YMZ_CONN_RECV_ERROR;
        }

        ws_log_info("recv size:%d, recv pos:%d, last finish:%d, handle pos:%d", flag, c->recv_pos, c->last_finish, c->handle_pos);
        curr += flag;
        size += flag;
        c->recv_pos += flag;
        msize -= flag;
        if (flag <= msize) {
            break;
        }
    }

    if ((rc != EAGAIN) && (!curr)) {
        ws_log_warn("recv state error, rc:%d, curr:%d", rc, curr);
        return YMZ_CONN_CLIENT_CLOSE;
    }

    return YMZ_CONN_RECV_OK;

}




