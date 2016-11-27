#include <ws_std.h>
#include <time.h>
#include <stdarg.h>

#include <sys/stat.h>

#include <ws_types.h>
#include <ws_time.h>

#include <ws_file.h>


ws_int32_t    ws_open_truncate_file(ws_char_t* path, ws_int32_t *pfd)
{
    struct stat						fsbuf;
    ws_int32_t                             fd;

    *pfd = 0;
    if (-1 == stat(path, &fsbuf)) {
        fd = ws_open_file(path, YMZ_FILE_WRITE, YMZ_FILE_CREATE, YMZ_FILE_DEFAULT_ACCESS);
    }
    else {
        fd = ws_open_file(path, YMZ_FILE_WRITE | YMZ_FILE_TRUNC, YMZ_FILE_CREATE, YMZ_FILE_DEFAULT_ACCESS);
    }


    if (fd <= 0) {
        printf("open file error, src:%s\n", path);
        return YMZ_ERROR;
    }

    *pfd = fd;

    return YMZ_OK;
}

size_t ws_write_file(ws_int32_t fd, char *buf, ws_int32_t size, ws_int32_t *offset)
{
	size_t  n, written;

	written = 0;


	for (;;) {
#ifndef _WIN32
		n = pwrite(fd, buf + written, size, offset);
#else
        fseek(fd, offset, 0);
        n = write(fd, buf + written, size);
#endif

		if (n == -1) {
			return YMZ_ERROR;
		}

		*offset += n;
		written += n;

		if ((size_t)n == size) {
			return YMZ_OK;
		}

		size -= n;
	}

	return YMZ_OK;
}