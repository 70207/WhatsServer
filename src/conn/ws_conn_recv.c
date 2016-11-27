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


#include <ws_conn_recv.h>

#define MIN_ITF		512


ws_int32_t ws_check_recv( ws_conn_t  *c )
{
	ws_mem_buf_t		 *recv_buf;
	char				 *buffer;
	char				 *pos;
	ws_int32_t					  size;
	ws_int32_t					  psize;

	recv_buf = c->recv_buf;
	buffer = recv_buf->data;

	if( c->read_buffer_size - c->recv_pos > MIN_ITF ){
		c->max_recv = c->read_buffer_size - c->recv_pos - MIN_ITF;
		return YMZ_OK;
	}
	

	if( c->recv_mark ){
		size = c->recv_pos - c->last_finish;
		if( size < 4 ){
			c->max_recv = 8 - size;
		}
		else{
			pos = buffer + c->last_finish;
			psize = *(ws_int32_t*)pos;
			c->max_recv = psize - size;
            ws_log_warn("mem not enough, conn:%d, last finish:%d, recv pos:%d, max buffer size:%d, header size:%d",
                c->index,
                c->last_finish, c->recv_pos, c->read_buffer_size, psize);
		}
		
		return YMZ_OK;
	}

	return YMZ_CONN_RECV_SHOULD_HANDLE;

}

ws_int32_t ws_check_handle( ws_conn_t* c, ws_char_t** pos, ws_int32_t* psize )
{
	ws_mem_buf_t		 *recv_buf;
	char				 *buffer;
	ws_int32_t					  size;
	ws_int32_t					  header;
	

	recv_buf = c->recv_buf;
	if (!recv_buf) {
		return YMZ_CONN_RECV_NO_HANDLE;
	}

	buffer = recv_buf->data;

	size = c->read_buffer_size - c->recv_pos;
	if( ( size <= MIN_ITF ) && (!c->recv_mark )
		&& ( c->handle_pos == c->recv_pos ) ){
			c->recv_pos = 0;
			c->handle_pos = 0;
			c->last_finish = 0;
			ws_log_info("recv pos move to 0 with conn:%d", c->index );
			return YMZ_CONN_RECV_NO_HANDLE;
	}

	size = c->last_finish -  c->handle_pos;
	if( size <= 0 ){
        ws_log_error("recv conn:%d, last finish is less than handle pos, last finish:%d, handle pos:%d",c->index,
            c->last_finish, c->handle_pos);
		return YMZ_CONN_RECV_NO_HANDLE;
	}

	
	*pos = buffer + c->handle_pos;
	header = *(ws_int32_t*)(*pos);
	if (header > size){
        ws_log_error("header is bigger than size, conn:%d, header:%d, size:%d", c->index, header, size);
		return YMZ_CONN_RECV_NO_HANDLE;
	}
	*psize = header;
	c->handle_pos += header;

	return YMZ_OK;

}


ws_int32_t ws_conn_recv( ws_conn_t   *c )
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
    ws_int32_t                   rtn;

    


	ws_log_access_debug("ws_conn_recv in maxrecv:%d, conn:%d", c->max_recv, c->index);
	pool = c->pool;
	recv_buf = c->recv_buf;
	
	size = c->recv_pos;
	all_size = size + c->max_recv;
    curr = 0;

	msize = c->max_recv;
	rc = 0;
    rtn = YMZ_CONN_RECV_OK;
	while( size < all_size ){
		flag = recv( c->s, recv_buf->data + size, msize, 0 );
		if( flag < 0 ){
			err = errno;
            if (err == 0) {
                ws_log_warn("recv close, rc:%d, curr:%d, conn:%d", rc, curr, c->index);
                rtn = YMZ_CONN_CLIENT_CLOSE;
                break;
            }
			if( err == EAGAIN ){
				rc = EAGAIN;
				ws_log_info( "recv again, recv_pos:%d, last_finish:%d, mark:%d", c->recv_pos, c->last_finish, c->recv_mark );
				break;
			}
			else if( err == EINTR ){
				continue;
			}

			ws_log_warn("recv error" );
			rtn = YMZ_CONN_RECV_ERROR;
            break;
		}
        else if(flag == 0) {
            ws_log_warn("recv close, rc:%d, curr:%d, conn:%d", rc, curr, c->index);
            rtn = YMZ_CONN_CLIENT_CLOSE;
            break;
        }
		
        ws_log_info("conn:%d,recv size:%d, recv pos:%d, last finish:%d, handle pos:%d", c->index,flag, c->recv_pos,
            c->last_finish, c->handle_pos);
        curr += flag;
		size += flag;
		c->recv_pos += flag;
		msize -= flag;
        if( flag <= msize ){
            break;
        }
	}
	
    if( ( rc != EAGAIN ) && ( !curr ) ){
		ws_log_warn("recv state error, rc:%d, curr:%d, conn:%d", rc, curr, c->index);
        return YMZ_CONN_CLIENT_CLOSE;
    }

	while(1){
		size = c->recv_pos - c->last_finish;
		if( size == 0 ){
			c->recv_mark = 0;
			break;
		}
		if( size  < 8 ){
			c->recv_mark = 1;
			break;
		}

		pos = recv_buf->data + c->last_finish;
		header = *(ws_int32_t*)pos;

		if ((header <= 8) || (header > YMZ_CONN_RECV_MAX_SIZE)) {
			ws_log_access_warn("error header:%d, conn:%d", header, c->index);
			return YMZ_CONN_CLIENT_CLOSE;
		}

        ws_log_debug("last finish header:%d, last finish:%d, recv pos:%d, conn:%d", header, c->last_finish, c->recv_pos,c->index);
        if (size > 16) {
            ws_log_debug("last finish cmd:%d", ((ws_int32_t*)pos)[2]);
        }

		if( size < header ){
			c->recv_mark = 1;
			break;
		}

		seqid = *(ws_int32_t*)(pos+4);
		ws_log_access_debug( "check seqid:%d", seqid );
		c->last_finish += header;
		if( c->last_finish == c->recv_pos ){
			c->recv_mark = 0;
			break;
		}
	}

	if( rc==EAGAIN){
		ws_log_access_debug("last finish:%d, recv_pos:%d, conn:%d", c->last_finish, c->recv_pos, c->index );
	}

    return rtn;
	
}

ws_int32_t ws_conn_recv_init( ws_conn_t *c )
{
	ws_mem_buf_t		 *recv_buf;

	recv_buf = c->recv_buf;

	recv_buf->pos = 0;

	return YMZ_CONN_RECV_OK;
}
