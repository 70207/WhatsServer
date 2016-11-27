#include <ws_std.h>
#include <errno.h>

#include <ws_event.h>
#include <ws_types.h>
#include <ws_mem.h>
#include <ws_cycle.h>
#include <ws_process_pipe.h>

ws_int32_t ws_process_pipe_read( ws_int32_t fd, ws_process_pipe_t* pp, ws_int32_t size )
{
	ws_int32_t					flag;
	flag = read( fd, pp, size );

	if( flag == -1 ){
		if( EAGAIN == flag ){
			return YMZ_AGAIN;
		}

		return YMZ_ERROR;
	}

	if( flag < sizeof( ws_process_pipe_t ) ){
		return YMZ_ERROR;
	}

	return YMZ_OK;
}

ws_int32_t ws_process_pipe_write( ws_int32_t fd, ws_process_pipe_t* pp, ws_int32_t size )
{
	ws_int32_t					flag;

	flag = write( fd, pp, size );

	if( flag == -1 ){
		if( errno == EAGAIN ){
			return YMZ_AGAIN;
		}

		return YMZ_ERROR;
	}

	return YMZ_OK;
}