#include <ws_std.h>
#include <ws_inet.h>
#include <errno.h>

#include <ws_types.h>
#include <ws_log.h>
#include <ws_mem.h>
#include <ws_event.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_log.h>


#include <ws_conn_websocket.h>
#include <ws_websocket.h>




#define MIN_ITF		512


ws_int32_t ws_conn_websocket_check_recv(ws_conn_t  *c)
{
    ws_mem_buf_t		 *recv_buf;
    ws_char_t			 *buffer;
    ws_char_t			 *pos;
    ws_int32_t			  size;
    ws_int32_t			  psize, payload_length;
    ws_int32_t           *data;
    ws_int64_t           *ldata;
    ws_int32_t            mask;

    recv_buf = c->recv_buf;
    buffer = recv_buf->data;

    if (c->read_buffer_size - c->recv_pos > MIN_ITF) {
        c->max_recv = c->read_buffer_size - c->recv_pos - MIN_ITF;
        return YMZ_OK;
    }


    if (c->recv_mark) {
        size = c->recv_pos - c->last_finish;
        if (size < 16) {
            c->max_recv = 16 - size;
        }
        else {
            data = buffer + c->last_finish;
            mask = (*data >> 7) > 0;
            payload_length = ((*data) & 0xfe00) >> 9;
            psize = 2;

            if (payload_length == 126) {
                payload_length = *data >> 16;
                psize = 4;
            }
            else if (payload_length == 127) {
                ldata = buffer + c->last_finish + 2;
                payload_length = *ldata;
                psize = 6;
            }
            psize += payload_length;
            if (mask) {
                psize += 4;
            }
            c->max_recv = psize - size;
            ws_log_warn("mem not enough, last finish:%d, recv pos:%d, max buffer size:%d, header size:%d",
                c->last_finish, c->recv_pos, c->read_buffer_size, psize);
        }

        return YMZ_OK;
    }

    return YMZ_CONN_RECV_SHOULD_HANDLE;

}




ws_int32_t ws_conn_websocket_recv(ws_conn_t   *c)
{
    ws_int32_t                   size;
    ws_int32_t                   curr;
    ws_int32_t                   flag;
    ws_int32_t                   err;
    ws_int32_t                   all_size;
    ws_int32_t					  msize;
    ws_mem_pool_t		         *pool;
    ws_mem_buf_t		         *recv_buf;
    ws_char_t*				      pos;
    ws_int32_t 				      psize, payload_length;
    ws_int32_t					  rc;
    ws_int32_t					  seqid;
    ws_int32_t                   *data;
    ws_int64_t                   *ldata;
    ws_int32_t                    mask;




    ws_log_access_debug("ws_conn_recv in maxrecv:%d", c->max_recv);
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
                ws_log_info("recv again, recv_pos:%d, last_finish:%d, mark:%d", c->recv_pos, c->last_finish, c->recv_mark);
                break;
            }
            else if (err == EINTR) {
                continue;
            }

            ws_log_warn("recv error");
            return YMZ_CONN_RECV_ERROR;
        }

        ws_log_info("recv size:%d, recv pos:%d, last finish:%d, handle pos:%d", flag, c->recv_pos,
            c->last_finish, c->handle_pos);
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

    while (1) {
        size = c->recv_pos - c->last_finish;
        if (size == 0) {
            c->recv_mark = 0;
            break;
        }
        if (size  < 16) {
            c->recv_mark = 1;
            break;
        }

        data = recv_buf->data + c->last_finish;
        mask = (*data >> 7) > 0;
        payload_length = ((*data) & 0xfe00) >> 9;
      
        psize = 2;
        if (payload_length == 126) {
            payload_length = *data >> 16;
            psize = 4;
        }
        else if (payload_length == 127) {
            ldata = recv_buf->data + c->last_finish + 2;
            payload_length = *ldata;
            psize = 6;
        }
        
        psize += payload_length;
        if (mask) {
            psize += 4;
        }

        if (psize < 20) {
            ws_log_access_warn("error packet size:%d", psize);
            return YMZ_CONN_CLIENT_CLOSE;
        }

        if (psize > YMZ_CONN_RECV_MAX_SIZE) {
            ws_log_access_warn("error packet size:%d", psize);
            return YMZ_CONN_CLIENT_CLOSE;
        }

        if (size < psize) {
            c->recv_mark = 1;
            break;
        }

        c->last_finish += psize;
        if (c->last_finish == c->recv_pos) {
            c->recv_mark = 0;
            break;
        }
    }

    if (rc == EAGAIN) {
        ws_log_access_debug("last finish:%d, recv_pos:%d", c->last_finish, c->recv_pos);
    }


    return YMZ_CONN_RECV_OK;

}




ws_int32_t ws_conn_websocket_check_handle(ws_conn_t* c, ws_char_t** pos, ws_int32_t* packetsize)
{
    ws_mem_buf_t		          *recv_buf;
    ws_char_t				      *buffer;
    ws_int32_t					   size, i;
    ws_int32_t					   psize, header, payload_length;
    ws_int32_t                    *idata;
    ws_uchar_t                    *req, *data;
    ws_int64_t                    *ldata;
    ws_int32_t                     mask;
    ws_uchar_t                     umask[4];


    recv_buf = c->recv_buf;
    if (!recv_buf) {
        return YMZ_CONN_RECV_NO_HANDLE;
    }

    buffer = recv_buf->data;

    size = c->read_buffer_size - c->recv_pos;
    if ((size <= MIN_ITF) && (!c->recv_mark)
        && (c->handle_pos == c->recv_pos)) {
        c->recv_pos = 0;
        c->handle_pos = 0;
        c->last_finish = 0;
        ws_log_info("recv pos move to 0 with conn:%d", c->index);
        return YMZ_CONN_RECV_NO_HANDLE;
    }

    size = c->last_finish - c->handle_pos;
    if (size <= 0) {
        ws_log_error("recv last finish is less than handle pos, last finish:%d, handle pos:%d",
            c->last_finish, c->handle_pos);
        return YMZ_CONN_RECV_NO_HANDLE;
    }

    
    data = idata = recv_buf->data + c->handle_pos;
    mask = (data[1] >> 7) > 0;
    payload_length = ((*idata) & 0xfe00) >> 9;
    header = 2;
    req = data + 2;
    if (payload_length == 126) {
        payload_length = *idata >> 16;
        req = data + 4;
        header = 4;
    }
    else if (payload_length == 127) {
        ldata = recv_buf->data + c->last_finish + 2;
        payload_length = *ldata;
        req = data + 6;
        header = 6;
    }

    if (mask) {
        memcpy(umask, data + header, 4);
        header += 4;
        req += 4;
    }

    psize = payload_length + header;
   

    if (psize < 20) {
        ws_log_access_warn("error package size:%d", psize);
        return YMZ_CONN_CLIENT_CLOSE;
    }

    if (psize > size) {
        ws_log_error("package size is bigger than size, package size:%d, size:%d", psize, size);
        return YMZ_CONN_RECV_NO_HANDLE;
    }

    

    *pos = req;

    *packetsize = payload_length;
    c->handle_pos += header;

    if (!mask) {
        return YMZ_OK;
    }

    for (i = 0; i < payload_length; ++i) {
        req[i] = req[i] ^ umask[i % 4];
    }

    return YMZ_OK;

}



ws_int32_t ws_conn_websocket_check_send_buf(ws_conn_t *c, ws_mem_buf_t *buf)
{
    ws_char_t					    *write_pos;
    ws_int32_t						 size;
    ws_int32_t                       package_length, header;
    ws_char_t                       *data;
    ws_int32_t                      *idata;
    ws_int64_t                      *ldata;
    ws_websocket_header_t           *rsp;


    if (buf->hd <= 0) {
        buf->hd = 0;
        return YMZ_OK;
    }


    if (c->send_wpos + buf->hd >= YMZ_CONN_SEND_BUF_SIZE) {
        ws_log_print_debug("check send buf failed, send wpos:%d, need append:%d, max:%d", c->send_wpos, buf->hd, YMZ_CONN_SEND_BUF_SIZE);
        return YMZ_ERROR;
    }

    write_pos = c->send_buf2 + c->send_wpos;
    rsp = data = idata = write_pos;

    data[0] = 0;
    size = buf->hd;
    if (size > 65535) {
        rsp->payload_len = 127;
        rsp->extend_payload_length.len_64 = size;
        header = 6;
    }
    else if (size > 125) {
        rsp->payload_len = 126;
        rsp->extend_payload_length.len_16 = size;
        header = 4;
    }
    else {
        rsp->payload_len = size;
        header = 2;
    }

    size += header;
    write_pos += header;
    memcpy(write_pos, buf->data, buf->hd);
    c->send_wpos += size;

    buf->hd = 0;

    return YMZ_OK;
}