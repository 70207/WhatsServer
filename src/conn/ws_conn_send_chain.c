
#include <string.h>
#include <errno.h>
#include <ws_std.h>
#include <ws_inet.h>

#include <ws_types.h>
#include <ws_mem.h>
#include <ws_log.h>
#include <ws_chain.h>
#include <ws_event_timer.h>
#include <ws_conn.h>
#include <ws_conn_send.h>
#include <ws_conn_send_chain.h>

#define YMZ_CONN_SEND_LIFE_TIME_DEFAULT					2

ws_int32_t ws_send( ws_int32_t fd, ws_uchar_t* buffer, ws_int32_t len, ws_int32_t* sent );
ws_int32_t ws_send_conn_chain( ws_int32_t fd, ws_chain_t* chain );

ws_int32_t ws_conn_send_chain( ws_conn_t* conn )
{
	ws_int32_t ret;
	ws_int32_t fd;
	ws_int32_t	outw;
	ws_event_t* rev;
	ws_event_t* wev;
	ws_chain_t* chain, *_chain;

	fd = conn->s;

	//printf( "fd:%d to write,chain:%llx\n", fd, conn->send_chain );

	chain = ws_chain_get_next( conn->send_chain );
	conn->send_chain = chain;
	rev = conn->rev;
	wev = conn->wev;


	if( !chain )
	{
		printf( "func:%s, fd:%d not chain\n",__FUNCTION__, fd );
		return -1;
	}

	outw = 0;
	while( chain && (!outw) )
	{
		ret = ws_send_conn_chain(  fd, chain );
		if( ret ){
			return ret;
		}

		ws_log_access_debug("send chain data:%d, chain size:%d, chain_pos:%d", chain->data, chain->io_len, chain->io_pos );
		//_chain = chain;
		chain = chain->next;
		/*conn->send_chain = chain;
		ws_chain_free_item( _chain );*/
		conn->chain_num--;
	}

	ws_chain_free( conn->send_chain );
	conn->send_chain = 0;
	conn->chain_num = 0;

	return ret;
}

ws_int32_t ws_send_conn_chain( ws_int32_t fd, ws_chain_t* chain )
{

	ws_int32_t len;
	ws_int32_t ret;
	ws_int32_t sent;
	ws_int32_t io_pos;

	io_pos = chain->io_pos;
	len = chain->io_len;
	len -= io_pos;
	if( len <= 0 )
	{
		return YMZ_OK;
	}

	//printf( "fd:%d, %s,len:%d\n", fd, __FUNCTION__, len );
	sent = 0;
	ret = ws_send( fd, chain->buf + io_pos,  len, &sent );
	io_pos += sent;
	chain->io_pos = io_pos;

	return ret;
}

ws_int32_t ws_send( ws_int32_t fd, ws_uchar_t* buffer, ws_int32_t len, ws_int32_t* sent )
{
	ws_int32_t ret;
	ws_int32_t err;
	ws_int32_t lsent;

	lsent = 0;
	while( lsent < len )
	{
	  // printf( "ws_send\n");
	   ret =  send( fd, buffer + lsent, len - lsent, 0 );
	   if( ret < 0 )
	   {
		
		err = errno;
		if( err == EAGAIN )
		{
			//printf( "fd:%d send again\n", fd);
			return YMZ_AGAIN;
		}
		else if( err == EINTR )
		{
			//printf( "fd:%d send intr\n", fd);
			continue;
		}
		else
		{
			//printf( "fd:%d send wrong:%d\n", fd, err);
		    return YMZ_ERROR;
		}
	  }

	//  printf( "fd:%d, send len:%d\n", fd, ret );
	
	  lsent += ret;
	  *sent = lsent;
	}
	
	return YMZ_OK;
}

