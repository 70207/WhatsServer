#ifndef _WS_FILE_H__
#define _WS_FILE_H__


#include <fcntl.h>
#include <ws_types.h>

#define ws_open_file(name, mode, create, access)                            \
    open((const ws_char_t *) name, mode|create, access)


#define ws_close_file( name )				\
	close( name )

ws_int32_t    ws_open_truncate_file(ws_char_t* path, ws_int32_t *pfd);

size_t ws_write_file(ws_int32_t fd, char *buf, ws_int32_t size, ws_int32_t *offset);

#define YMZ_FILE_APPEND				(O_WRONLY|O_APPEND)
#define YMZ_FILE_CREATE_OR_OPEN		(O_CREAT|O_APPEND)
#define YMZ_FILE_CREATE				O_CREAT
#define YMZ_FILE_WRITE				O_WRONLY
#define YMZ_FILE_TRUNC				O_TRUNC
#define YMZ_FILE_DEFAULT_ACCESS		0644




#endif //_WS_FILE_H__
