// mislog.cpp : 定义控制台应用程序的入口点。

#include <ws_std.h>
#include <time.h>
#include <stdarg.h>


#include <sys/stat.h>

#include <ws_types.h>
#include <ws_time.h>
#include <ws_log.h>
#include <ws_log_core.h>
#include <ws_file.h>


enum{
	YMZ_LOG_TYPE_DEBUG = 0,
	YMZ_LOG_TYPE_INFO,
	YMZ_LOG_TYPE_WARN,
	YMZ_LOG_TYPE_ERROR,
	YMZ_LOG_TYPE_REPORT,
};

#define YMZ_LOG_SIZE_LIMIT       67108864
#define YMZ_LOG_FILE_NO_LIMIT	5

#define YMZ_LOG_MODE_CYCLE       0
#define YMZ_LOG_MODE_TIME        1

static ws_int32_t ws_log_error_fd = 0;
static ws_int32_t ws_log_access_fd = 0;
static ws_int32_t ws_log_sys_fd = 0;
static ws_int32_t ws_log_report_fd = 0;

static ws_int32_t ws_log_error_size = 0;
static ws_int32_t ws_log_access_size = 0;
static ws_int32_t ws_log_sys_size = 0;
static ws_int32_t ws_log_report_size = 0;

static ws_int32_t ws_log_error_file_no = 0;
static ws_int32_t ws_log_access_file_no = 0;
static ws_int32_t ws_log_sys_file_no = 0;
static ws_int32_t ws_log_report_file_no = 0;

static ws_int32_t ws_log_level = 0;

ws_int32_t ws_log_real( ws_int32_t wfd, ws_int32_t pos, ws_int32_t level, const ws_char_t* format, va_list arg_list );
ws_int32_t ws_log_real_with_print(ws_int32_t wfd, ws_int32_t pos, ws_int32_t level, const ws_char_t* format, va_list arg_list);
ws_int32_t ws_log_core( va_list arg_list, const ws_char_t* format, u_char* buffer );

static char  path_str[ 256 ];
static ws_int32_t   path_len = 0;

ws_int32_t ws_log_init()
{
	ws_int32_t								error_fd;
	ws_int32_t								access_fd;
	ws_int32_t								sys_fd;
	ws_int32_t								report_fd;
	
	char							buffer[ 256 ];
	ws_int32_t								len;
	const ws_char_t*						path;

	path = "./";
	
	len = 0;
	while( *path ){
		buffer[ len++ ] = *path;
		path++;
	}

	ws_char_t* pos = buffer + len;

	strcpy( pos, "/ws_error_log" );
	error_fd = ws_open_file( buffer, YMZ_FILE_WRITE, YMZ_FILE_CREATE_OR_OPEN, YMZ_FILE_DEFAULT_ACCESS );
	if( error_fd <= 0 ){
		printf( "error log :%s could not be opened\n", buffer );
		return YMZ_ERROR;
	}
	
	strcpy( pos, "/ws_access_log");
	access_fd = ws_open_file( buffer, YMZ_FILE_WRITE, YMZ_FILE_CREATE_OR_OPEN, YMZ_FILE_DEFAULT_ACCESS );
	if( access_fd <= 0 ){
		printf( "access log :%s could not be opened\n", buffer );
		return YMZ_ERROR;
	}

	strcpy( pos, "/ws_sys_log");
	sys_fd = ws_open_file( buffer, YMZ_FILE_WRITE, YMZ_FILE_CREATE_OR_OPEN, YMZ_FILE_DEFAULT_ACCESS );
	if( sys_fd <= 0 ){
		printf( "sys log :%s could not be opened\n", buffer );
		return YMZ_ERROR;
	}

	strcpy(pos, "/ws_report_log");
	report_fd = ws_open_file(buffer, YMZ_FILE_WRITE, YMZ_FILE_CREATE_OR_OPEN, YMZ_FILE_DEFAULT_ACCESS);
	if (report_fd <= 0){
		printf("report log :%s could not be opened\n", buffer);
		return YMZ_ERROR;
	}



	ws_log_error_fd = error_fd;
	ws_log_access_fd = access_fd;
	ws_log_sys_fd = sys_fd;
	ws_log_report_fd = report_fd;

	return YMZ_OK;
}

static ws_int32_t ws_log_mode = 0;

ws_int32_t ws_log_set_mode(ws_int32_t mode)
{
    ws_log_mode = mode;
    return YMZ_OK;
}

ws_int32_t ws_log_set_level(ws_int32_t level)
{
    ws_log_level = level;
    return YMZ_OK;
}

ws_int32_t ws_log_set_access( const ws_char_t* path )
{
	ws_int32_t								access_fd;

	access_fd = ws_open_file( path, YMZ_FILE_WRITE, YMZ_FILE_CREATE_OR_OPEN, YMZ_FILE_DEFAULT_ACCESS );
	if( access_fd <= 0 ){
		printf( "access log :%s could not be opened\n", path );
		return YMZ_ERROR;
	}

	if( ws_log_access_fd ){
		ws_close_file( ws_log_access_fd );
	}

	ws_log_access_fd = access_fd;

	return YMZ_OK;

}

ws_int32_t ws_log_set_error( const ws_char_t* path )
{
	ws_int32_t								error_fd;

	error_fd = ws_open_file( path, YMZ_FILE_WRITE, YMZ_FILE_CREATE_OR_OPEN, YMZ_FILE_DEFAULT_ACCESS );
	if( error_fd <= 0 ){
		printf( "error log :%s could not be opened\n", path );
		return YMZ_ERROR;
	}

	if( ws_log_error_fd ){
		ws_close_file( ws_log_error_fd );
	}

	ws_log_error_fd = error_fd;

	return YMZ_OK;
}

ws_int32_t ws_log_set_report(const ws_char_t* path)
{
	ws_int32_t								error_fd;

	error_fd = ws_open_file(path, YMZ_FILE_WRITE, YMZ_FILE_CREATE_OR_OPEN, YMZ_FILE_DEFAULT_ACCESS);
	if (error_fd <= 0){
		printf("error log :%s could not be opened\n", path);
		return YMZ_ERROR;
	}

	if (ws_log_report_fd){
		ws_close_file(ws_log_report_fd);
	}

	ws_log_report_fd = error_fd;

	return YMZ_OK;
}


ws_int32_t ws_log_turn_access()
{
	ws_char_t*							buffer;
	ws_char_t*							pos;
	ws_int32_t						access_fd;
	ws_int32_t						size;
	struct stat						fsbuf;

	buffer = path_str;
	access_fd = 0;
	pos = buffer + path_len;
    if (path_len != 0) {
        *pos++ = '/';
    }
    switch (ws_log_mode) {
    case YMZ_LOG_MODE_CYCLE:

        sprintf(pos, "ws_access_log.%d", ws_log_access_file_no);
        ws_log_access_file_no++;
        if (ws_log_access_file_no > YMZ_LOG_FILE_NO_LIMIT) {
            ws_log_access_file_no = 0;
        }
        break;
    case YMZ_LOG_MODE_TIME:
        sprintf(pos, "ws_access_log.%s", ws_cached_time_format_yyyyMMddhhmmss.data);
        break;
    default:
        printf("access  log mode:%d is error\n", ws_log_mode);
        sprintf(pos, "ws_access_log.%d", ws_log_access_file_no);
        ws_log_access_file_no++;
        if (ws_log_access_file_no > YMZ_LOG_FILE_NO_LIMIT) {
            ws_log_access_file_no = 0;
        }

        break;

    }
	
    if (YMZ_OK != ws_open_truncate_file(buffer, &access_fd)) {
        printf("access log :%s could not be opened\n", buffer);
        return YMZ_ERROR;
    }

	if (ws_log_access_fd){
		close(ws_log_access_fd);
	}

	ws_log_access_fd = access_fd;
	ws_log_access_size = 0;

	return YMZ_OK;

}

ws_int32_t ws_log_turn_error()
{
	ws_char_t*							buffer;
	ws_char_t*							pos;
	ws_int32_t						error_fd;
	ws_int32_t						size;
	struct stat						fsbuf;

	buffer = path_str;
	error_fd = 0;
	pos = buffer + path_len;

    if (path_len != 0) {
        *pos++ = '/';
    }

	
    switch (ws_log_mode) {
        case YMZ_LOG_MODE_CYCLE:
            sprintf(pos, "ws_error_log.%d", ws_log_error_file_no);
            ws_log_error_file_no++;
            if (ws_log_error_file_no > YMZ_LOG_FILE_NO_LIMIT) {
                ws_log_error_file_no = 0;
            }
            break;
        case YMZ_LOG_MODE_TIME:
            sprintf(pos, "ws_error_log.%s", ws_cached_time_format_yyyyMMddhhmmss.data);
            break;
        default:
            printf("error  log mode:%d is error\n", ws_log_mode);
            sprintf(pos, "ws_error_log.%d", ws_log_error_file_no);
            ws_log_error_file_no++;
            if (ws_log_error_file_no > YMZ_LOG_FILE_NO_LIMIT) {
                ws_log_error_file_no = 0;
            }
            break;
    }

    if (YMZ_OK != ws_open_truncate_file(buffer, &error_fd)) {
        printf("error log :%s could not be opened\n", buffer);
        return YMZ_ERROR;
    }

	if (ws_log_error_fd){
		close(ws_log_error_fd);
	}


	ws_log_error_fd = error_fd;
	ws_log_error_size = 0;


	return YMZ_OK;
}

ws_int32_t ws_log_turn_sys()
{
	ws_char_t*							buffer;
	ws_char_t*							pos;
	ws_int32_t						sys_fd;
	ws_int32_t						size;
	struct stat						fsbuf;
	
	buffer = path_str;
	sys_fd = 0;
	pos = buffer + path_len;
    if (path_len != 0) {
        *pos++ = '/';
    }
	
    switch (ws_log_mode) {
    case YMZ_LOG_MODE_CYCLE:
        sprintf(pos, "ws_sys_log.%d", ws_log_sys_file_no);
        ws_log_sys_file_no++;
        if (ws_log_sys_file_no > YMZ_LOG_FILE_NO_LIMIT) {
            ws_log_sys_file_no = 0;
        }
        break;
    case YMZ_LOG_MODE_TIME:
        sprintf(pos, "ws_sys_log.%s", ws_cached_time_format_yyyyMMddhhmmss.data);
        break;
    default:
        printf("sys  log mode:%d is error\n", ws_log_mode);
        sprintf(pos, "ws_sys_log.%d", ws_log_sys_file_no);
        ws_log_sys_file_no++;
        if (ws_log_sys_file_no > YMZ_LOG_FILE_NO_LIMIT) {
            ws_log_sys_file_no = 0;
        }
        break;
    }

    if (YMZ_OK != ws_open_truncate_file(buffer, &sys_fd)) {
        printf("error log :%s could not be opened\n", buffer);
        return YMZ_ERROR;
    }

	if (ws_log_sys_fd){
		close(ws_log_sys_fd);
	}


	ws_log_sys_fd = sys_fd;
	ws_log_sys_size = 0;

	return YMZ_OK;
}

ws_int32_t ws_log_turn_report()
{
	ws_char_t*							buffer;
	ws_char_t*							pos;
	ws_int32_t						report_fd;
	struct stat						fsbuf;
	ws_int32_t						size;


	buffer = path_str;
	report_fd = 0; 
	pos = buffer + path_len;
    if (path_len != 0) {
        *pos++ = '/';
    }
	
    switch (ws_log_mode) {
    case YMZ_LOG_MODE_CYCLE:
        sprintf(pos, "ws_report_log.%d", ws_log_report_file_no);
        ws_log_report_file_no++;
        if (ws_log_report_file_no > YMZ_LOG_FILE_NO_LIMIT) {
            ws_log_report_file_no = 0;
        }
        break;
    case YMZ_LOG_MODE_TIME:
        sprintf(pos, "ws_report_log.%s", ws_cached_time_format_yyyyMMddhhmmss.data);
        break;
    default:
        printf("report  log mode:%d is error\n", ws_log_mode);
        sprintf(pos, "ws_report_log.%d", ws_log_report_file_no);
        ws_log_report_file_no++;
        if (ws_log_report_file_no > YMZ_LOG_FILE_NO_LIMIT) {
            ws_log_report_file_no = 0;
        }
        break;
    }

    if (YMZ_OK != ws_open_truncate_file(buffer, &report_fd)) {
        printf("error log :%s could not be opened\n", buffer);
        return YMZ_ERROR;
    }

	if (ws_log_report_fd){
		close(ws_log_report_fd);
	}

	ws_log_report_fd = report_fd;
	ws_log_report_size = 0;

	return YMZ_OK;
}

ws_int32_t ws_log_set_path( const ws_char_t* path )
{
	ws_int32_t								error_fd;
	ws_int32_t								access_fd;
	ws_int32_t								sys_fd;
	
	ws_char_t*							    buffer;
	ws_int32_t								len;

	
	buffer = path_str;
	len = 0;
	while( *path ){
		buffer[ len++ ] = *path;
		path++;
	}

	path_len = len;
	if( ws_log_turn_access() ){
		return YMZ_ERROR;
	}
	if( ws_log_turn_error() ){
		return YMZ_ERROR;
	}

	if( ws_log_turn_sys() ){
		return YMZ_ERROR;
	}

	if (ws_log_turn_report()){
		return YMZ_ERROR;
	}

	

	return YMZ_OK;
}

ws_int32_t ws_log_set_path_for_printf()
{
	ws_log_error_fd = 0;
	ws_log_error_size = 0;
	ws_log_access_fd = 0;
	ws_log_access_size = 0;
	ws_log_sys_fd = 0;
	ws_log_sys_size = 0;
	ws_log_report_fd = 0;
	ws_log_report_size = 0;
    
    return YMZ_OK;
}

ws_int32_t ws_log_close()
{
	if( ws_log_error_fd ){
		ws_close_file( ws_log_error_fd );
		ws_log_error_fd = 0;
	}

	if( ws_log_access_fd ){
		ws_close_file( ws_log_access_fd );
		ws_log_access_fd = 0;
	}

	if( ws_log_sys_fd ){
		ws_close_file( ws_log_sys_fd );
		ws_log_sys_fd = 0;
	}

	if (ws_log_report_fd){
		ws_close_file(ws_log_report_fd);
		ws_log_report_fd = 0;
	}

	return 0;
}

ws_int32_t ws_log_core( va_list arg_list, const ws_char_t* format, u_char* buffer );



ws_int32_t ws_log_real( ws_int32_t wfd, ws_int32_t pos, ws_int32_t level, const ws_char_t* format, va_list arg_list )
{
	u_char				buffer[ YMZ_LOG_BUFFER_DEFAULT ];
	u_char*				p;
	ws_int32_t					size;
	ws_int32_t					rc;

	p = buffer;

	memcpy(p, ws_cached_err_log_utime.data, ws_cached_err_log_utime.size);

	p += ws_cached_err_log_utime.size;
	switch( level ){
	case YMZ_LOG_TYPE_DEBUG:
		size = sizeof( " [debug] " ) - 1;
		memcpy( p, " [debug] ", size );
		break;
	case YMZ_LOG_TYPE_INFO:
		size =  sizeof( " [info] " ) - 1;
		memcpy( p, " [info] ", size );
		break;
	case YMZ_LOG_TYPE_WARN:
		size = sizeof( " [warn] " ) - 1;
		memcpy( p, " [warn] ", size );
		break;
	case YMZ_LOG_TYPE_ERROR:
		size = sizeof( " [error] " ) - 1;
		memcpy( p, " [error] ", size );
		break;
	default:
		size = 0;
		return 0;
	}
	
	p += size;

	rc = ws_log_core( arg_list, format, p );

	size += ws_cached_err_log_utime.size;
	size += rc;
    if (size >= YMZ_LOG_BUFFER_DEFAULT) {
        printf("log buf is too big!");
        buffer[YMZ_LOG_BUFFER_DEFAULT - 1] = 0;
        printf("log buf:%s", buffer);
        return 0;
    }
	buffer[size] = 0;
	if( wfd ){
#ifndef _WIN32
		rc = pwrite( wfd, buffer, size + 1, pos);
#else
        rc = write(wfd, buffer, size + 1);
#endif
		//rc = ws_write_file(wfd, buffer, size + 1, ws_int32_t *offset);
	}
	else{
		printf(buffer);
	}

	return size + 1;
}

ws_int32_t ws_log_real_with_print(ws_int32_t wfd, ws_int32_t pos, ws_int32_t level, const ws_char_t* format, va_list arg_list)
{
	u_char				buffer[YMZ_LOG_BUFFER_DEFAULT];
	u_char*				p;
	ws_int32_t					size;
	ws_int32_t					rc;

	p = buffer;

	memcpy(p, ws_cached_err_log_utime.data, ws_cached_err_log_utime.size);

	p += ws_cached_err_log_utime.size;
	switch (level){
	case YMZ_LOG_TYPE_DEBUG:
		size = sizeof(" [debug] ") - 1;
		memcpy(p, " [debug] ", size);
		break;
	case YMZ_LOG_TYPE_INFO:
		size = sizeof(" [info] ") - 1;
		memcpy(p, " [info] ", size);
		break;
	case YMZ_LOG_TYPE_WARN:
		size = sizeof(" [warn] ") - 1;
		memcpy(p, " [warn] ", size);
		break;
	case YMZ_LOG_TYPE_ERROR:
		size = sizeof(" [error] ") - 1;
		memcpy(p, " [error] ", size);
		break;
	default:
		size = 0;
		return 0;
	}

	p += size;

	rc = ws_log_core(arg_list, format, p);

	size += ws_cached_err_log_utime.size;
	size += rc;
	buffer[size] = 0;
	if (wfd){
#ifndef _WIN32
        rc = pwrite(wfd, buffer, size + 1, pos);
#else
        rc = write(wfd, buffer, size + 1);
#endif
		//rc = ws_write_file(wfd, buffer, size + 1, ws_int32_t *offset);
	}

	printf(buffer);
	

	return size + 1;
}

ws_int32_t ws_log_core( va_list arg_list, const ws_char_t* format, u_char* buffer )
{
	char			convert[20];
    ws_uchar_t   uc[20];
    
    static const ws_char_t  xxv[] = { '0','1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e','f' };

	ws_int32_t				i,j;
	ws_int32_t				has;
	const ws_char_t*		stfor;
	ws_char_t*			pos;
	ws_int32_t				d_v;
	char			d_v_m;
	ws_uint32_t    u_v;
	long			l_v;
	long long       ll_v;
	char			ll_v_m;
	char			c_v;
	ws_char_t*			s_v;
    unsigned long long *pv;

	
	stfor = format;
	pos = buffer;
	has = 0;


	while( *stfor )
	{
		if( has )
		{
			ws_int32_t m = 0;
			ws_int32_t i = 0;
			switch( *stfor )
			{
			case 'd':
				d_v = va_arg( arg_list,ws_int32_t );
				i = 10;
				j = 0;
				if(d_v < 0)
				{
					d_v 	= -d_v;
					d_v_m 	= 1;
				}
				else
				{
					d_v_m 	= 0;
				}

				while (i--)
				{
					convert[i] = d_v%10;
					d_v /= 10;
				}
				while (j < 9 && convert[j] == 0){
					j++;
				}

				if(d_v_m)
				{
					*pos++ = '-';
				}
				for (;j<10;j++)
				{
					*pos = convert[j] + 48;
					++pos;
				}
				break;
			case 'l':
				stfor+= 2;
				switch(*stfor ){
				case 'd':
					i = 20;
					j = 0;
					ll_v = va_arg( arg_list, long);
					if(ll_v < 0)
					{
						ll_v 	= -ll_v;
						ll_v_m	= 1;
					}
					else
					{
						ll_v_m	= 0;
					}

					while (i--)
					{
						convert[i] = ll_v%10;
						ll_v /= 10;

					}
					while (convert[j] == 0 && j < 19){
						j++;
					}

					if(ll_v_m)
					{
						*pos++ = '-';
					}

					for (;j<20;j++)
					{
						*pos = convert[j] + 48;
						++pos;
					}
					break;
				case 'u':
					i = 20;
					j = 0;
					ll_v = va_arg( arg_list, unsigned long long );
					while (i--)
					{
						convert[i] = ll_v%10;
						ll_v /= 10;

					}
					while (convert[j] == 0 && j < 19){
						j++;
					}
					for (;j<20;j++)
					{
						*pos = convert[j] + 48;
						++pos;
					}
					break;
                case 'x':
                    ll_v = va_arg(arg_list, unsigned long long);
                    *pos++ = 48;
                    *pos++ = 'x';
                     pv = uc;
                    *pv = ll_v;
                    for (i = 0; i < 7; ++i) {
                        j = uc[i] / 16;
                        *pos++ = xxv[j];
                        j = uc[i] % 16;
                        *pos++ = xxv[j];
                    }
                    break;
				default:
					printf( "the ll printf is wrong\n" );
					return YMZ_ERROR;
				}
				break;
			case 'u':
				u_v = va_arg( arg_list,ws_uint32_t );
				i = 10;
				j = 0;
				while (i--)
				{
					convert[i] = u_v%10;
					u_v /= 10;
				}
				while (convert[j] == 0 && j < 9){
					j++;
				}
				for (;j<10;j++)
				{
					*pos = convert[j] + 48;
					++pos;
				}
				break;
			case 's':
				s_v = va_arg( arg_list, ws_char_t* );
				while( *s_v ){
					*pos++ = *s_v;
					++s_v;
				}
				break;
			case 'c':
				c_v = va_arg( arg_list, char );
				*pos++ = c_v;
				break;

			}

			has = 0;
			stfor++;
			continue;
		}
		if(  *stfor == '%' )
		{
			has = 1;
			++stfor;
			continue;
		}
		else
		{
			*pos++ = *stfor;
		}

		++stfor;


	}
	*pos = '\n';

	return (ws_uchar_t*)pos - buffer + 1;

}

#define ws_log_start( format, level, wfd, pos ) \
	va_list arg_list; \
	if( level < ws_log_level ){		\
		return YMZ_OK;					\
	}									\
	va_start( arg_list, format ); \
	size = ws_log_real( wfd, pos, level, format, arg_list ); 

#define ws_log_start_print( format, level, wfd, pos ) \
	va_list arg_list; \
	if( level < ws_log_level ){		\
		return YMZ_OK;					\
		}									\
	va_start( arg_list, format ); \
	size = ws_log_real_with_print( wfd, pos, level, format, arg_list ); 

#define ws_log_end \
	va_end( arg_list );

#define ws_log_check_error \
		ws_log_error_size += size;  \
		if( ws_log_error_size > YMZ_LOG_SIZE_LIMIT ) {  \
			ws_log_turn_error(); \
		}    \

#define ws_log_check_access \
		ws_log_access_size += size;  \
		if( ws_log_access_size > YMZ_LOG_SIZE_LIMIT ) { \
			ws_log_turn_access();		\
		}  \

#define ws_log_check_sys \
		ws_log_sys_size+= size; \
		if( ws_log_sys_size > YMZ_LOG_SIZE_LIMIT ) { \
			ws_log_turn_sys(); \
		} \

#define ws_log_check_report \
		ws_log_report_size+= size; \
		if( ws_log_report_size > YMZ_LOG_SIZE_LIMIT ) { \
			ws_log_turn_report(); \
				} \

ws_int32_t ws_log_real_debug( const ws_char_t* format, ... )
{
	ws_int32_t		size;
	ws_log_start( format, YMZ_LOG_TYPE_DEBUG, ws_log_error_fd, ws_log_error_size);
	ws_log_end;

	ws_log_check_error;
	return 0;
}



ws_int32_t ws_log_real_info( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_INFO, ws_log_error_fd, ws_log_error_size);
	ws_log_end;

	ws_log_check_error;
	return 0;

}


ws_int32_t ws_log_real_warn( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_WARN, ws_log_error_fd, ws_log_error_size);
	ws_log_end;

	ws_log_check_error;
	return 0;
}

ws_int32_t ws_log_real_error( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_ERROR, ws_log_error_fd, ws_log_error_size);
	ws_log_end;

	ws_log_check_error;
	return 0;
}

ws_int32_t ws_log_print_real_debug(const ws_char_t* format, ...)
{
	ws_int32_t		size;
#ifdef YMZ_PRINT_LOG
	ws_log_start_print(format, YMZ_LOG_TYPE_DEBUG, ws_log_report_fd, ws_log_report_size);
#else
	ws_log_start(format, YMZ_LOG_TYPE_DEBUG, ws_log_report_fd, ws_log_report_size);
#endif
	ws_log_end;

	ws_log_check_report;
	return 0;
}

ws_int32_t ws_log_print_real_info(const ws_char_t* format, ...)
{
	ws_int32_t		size;
#ifdef YMZ_PRINT_LOG
	ws_log_start_print(format, YMZ_LOG_TYPE_INFO, ws_log_report_fd, ws_log_report_size);
#else
	ws_log_start(format, YMZ_LOG_TYPE_INFO, ws_log_report_fd, ws_log_report_size);
#endif
	ws_log_end;
	ws_log_check_report;

	return 0;
}

ws_int32_t ws_log_print_real_warn(const ws_char_t* format, ...)
{
	ws_int32_t		size;
#ifdef YMZ_PRINT_LOG
	ws_log_start_print(format, YMZ_LOG_TYPE_WARN, ws_log_report_fd, ws_log_report_size);
#else
	ws_log_start(format, YMZ_LOG_TYPE_WARN, ws_log_report_fd, ws_log_report_size);
#endif
	ws_log_end;
	ws_log_check_report;

	return 0;
}

ws_int32_t ws_log_print_real_error(const ws_char_t* format, ...)
{
	ws_int32_t		size;
#ifdef YMZ_PRINT_LOG
	ws_log_start_print(format, YMZ_LOG_TYPE_ERROR, ws_log_report_fd, ws_log_report_size);
#else
	ws_log_start(format, YMZ_LOG_TYPE_ERROR, ws_log_report_fd, ws_log_report_size);
#endif
	ws_log_end;
	ws_log_check_report;

	return 0;
}


ws_int32_t ws_log_access_real_debug( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_DEBUG, ws_log_access_fd, ws_log_access_size );
	ws_log_end;

	ws_log_check_access;

	return 0;
}

ws_int32_t ws_log_access_real_info( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_INFO, ws_log_access_fd, ws_log_access_size);
	ws_log_end;

	ws_log_check_access;

	return 0;
}

ws_int32_t ws_log_access_real_warn( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_WARN, ws_log_access_fd, ws_log_access_size);
	ws_log_end;

	ws_log_check_access;

	return 0;

}
ws_int32_t ws_log_access_real_error( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_ERROR, ws_log_access_fd, ws_log_access_size);
	ws_log_end;

	ws_log_check_access;
	return 0;
}


ws_int32_t ws_log_sys_real_debug( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_DEBUG, ws_log_sys_fd, ws_log_sys_size );
	ws_log_end;

	ws_log_check_sys;
	return 0;
}

ws_int32_t ws_log_sys_real_info( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_INFO, ws_log_sys_fd, ws_log_sys_size);
	ws_log_end;

	ws_log_check_sys;
	return 0;
}

ws_int32_t ws_log_sys_real_warn( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_WARN, ws_log_sys_fd, ws_log_sys_size);
	ws_log_end;

	ws_log_check_sys;
	return 0;

}
ws_int32_t ws_log_sys_real_error( const ws_char_t* format, ... )
{
	ws_int32_t				size;
	ws_log_start( format, YMZ_LOG_TYPE_ERROR, ws_log_sys_fd, ws_log_sys_size);
	ws_log_end;

	ws_log_check_sys;
	return 0;
}
