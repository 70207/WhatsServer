#ifndef _WS_PROCESS_PIPE_H__
#define _WS_PROCESS_PIPE_H__

#ifdef _WIN32
typedef ws_int32_t pid_t;
#endif

#include <ws_types.h>

typedef struct ws_process_pipe_s ws_process_pipe_t;

struct ws_process_pipe_s
{
	ws_uint32_t	cmd;
	pid_t			pid;
	ws_int32_t				slot;
	ws_int32_t				data;
};



ws_int32_t ws_process_pipe_read( ws_int32_t fd, ws_process_pipe_t* pp, ws_int32_t size );
ws_int32_t ws_process_pipe_write( ws_int32_t fd, ws_process_pipe_t* pp, ws_int32_t size );

extern  ws_process_pipe_t  ws_process_pipe;

#endif //_WS_PROCESS_PIPE_H__
