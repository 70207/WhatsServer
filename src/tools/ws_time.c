//
//  ws_time.c
//  whatsserverz
//
//  Created by peter chain on 13-6-8.
//  Copyright (c) 2013年 peter chain. All rights reserved.
//

#include <ws_std.h>
#include <ws_inet.h>


#include <time.h>
#include <ws_types.h>
#include <ws_time.h>

#ifdef  _WIN32
#include <Windows.h>
#endif


static ws_uchar_t            cached_err_log_time[sizeof("1970/09/28 12:00:00")];
static ws_uchar_t			cached_err_log_utime[sizeof("1970/09/28 12:00:00.000000")];
static ws_uchar_t            cached_time_format_yyyyMMddhhmmss[sizeof("19700928120000")];
static ws_uchar_t            cached_time_format_yyyy_MM_dd_hh_mm_ss[sizeof("1970-09-28 12:00:00")];
static ws_uchar_t            cached_time_format_yyyy_MMdd[sizeof("1970_0928")];


volatile ws_str_t				ws_cached_err_log_time;
volatile ws_str_t				ws_cached_err_log_utime;
volatile ws_str_t               ws_cached_time_format_yyyyMMddhhmmss;
volatile ws_str_t               ws_cached_time_format_yyyy_MM_dd_hh_mm_ss;
volatile ws_str_t               ws_cached_time_format_yyyy_MMdd;

ws_msec_t	ws_current_usec = 0;
ws_msec_t	ws_current_msec = 0;
ws_sec_t	ws_current_sec = 0;
ws_int32_t	ws_current_times = 0;
ws_int32_t  ws_last_static_msec = 0;

ws_int32_t  ws_current_year = 0;
ws_int32_t  ws_current_month = 0;
ws_msec_t   ws_current_year_month = 0;

void ws_time_init()
{
	ws_cached_err_log_time.size = sizeof("1970/09/28 12:00:00") - 1;
	ws_cached_err_log_utime.size = sizeof("1970/09/28 12:00:00.000000") - 1;
    ws_cached_time_format_yyyyMMddhhmmss.size = sizeof("19700928120000") - 1;
	ws_cached_time_format_yyyy_MM_dd_hh_mm_ss.size = sizeof("1970-09-28 12:00:00") - 1;
    ws_cached_time_format_yyyy_MMdd.size = sizeof("1970_0928") - 1;
	ws_time_update();
}

void ws_time_update()
{
    time_t			sec;
	struct tm		tm, *ptm;
	ws_uint32_t	msec;
	struct timeval  tv;

	ws_uchar_t*  p;
    
#ifndef _WIN32
	gettimeofday( &tv, NULL );
	sec = tv.tv_sec;
	msec = tv.tv_usec / 1000;
#else
    SYSTEMTIME systime;
    GetLocalTime(&systime);
    sec = systime.wSecond;
    msec = systime.wMilliseconds;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
#endif
	ws_current_msec = ( ws_msec_t )sec * 1000 + msec;
	ws_current_usec = (ws_msec_t)sec * 1000000 + msec;
	sec = time(0);
	ws_current_sec = sec;

	
	ptm = localtime( &sec );
	tm = *ptm;

	tm.tm_mon++;
	tm.tm_year += 1900;

    ws_current_year = tm.tm_year;
    ws_current_month = tm.tm_mon;

    ws_current_year_month = ws_current_year * 100 + ws_current_month;
    
    {
        p = cached_err_log_time;

        sprintf(p, "%4d/%02d/%02d %02d:%02d:%02d",
            tm.tm_year, tm.tm_mon,
            tm.tm_mday, tm.tm_hour,
            tm.tm_min, tm.tm_sec);

        ws_cached_err_log_time.data = p;
    }
    {
        p = cached_err_log_utime;

        sprintf(p, "%4d/%02d/%02d %02d:%02d:%02d.%06d",
            tm.tm_year, tm.tm_mon,
            tm.tm_mday, tm.tm_hour,
            tm.tm_min, tm.tm_sec,
            tv.tv_usec);

        ws_cached_err_log_utime.data = p;
    }

    {
        p = cached_time_format_yyyyMMddhhmmss;
        sprintf(p, "%4d%02d%02d%02d%02d%02d",
            tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

        ws_cached_time_format_yyyyMMddhhmmss.data = p;
    }

    {
        p = cached_time_format_yyyy_MMdd;
        sprintf(p, "%4d_%02d%02d", tm.tm_year, tm.tm_mon, tm.tm_mday);

        ws_cached_time_format_yyyy_MMdd.data = p;
    }

	{
		p = cached_time_format_yyyy_MM_dd_hh_mm_ss;
		sprintf(p, "%4d-%02d-%02d %02d:%02d:%02d",
			tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		ws_cached_time_format_yyyy_MM_dd_hh_mm_ss.data = p;
	}
    if (ws_current_msec > ws_last_static_msec) {
        ws_current_times = 0;
        ws_last_static_msec = ws_current_msec;
    }

}

ws_int64_t	ws_time_times_get(ws_msec_t timer)
{
	ws_int64_t			times;

	ws_current_times++;
	times = timer * 10000 + ws_current_times;

	return times;
}

ws_int64_t	ws_time_times_get_static(ws_msec_t timer)
{
	ws_int64_t			times;

	times = timer * 10000 + ws_current_times;

	return times;
}

ws_int64_t	ws_current_times_get()
{
	return ws_current_msec * 10000;
}

ws_int64_t  ws_time_mtime_get(ws_int64_t times)
{
	return ws_current_times / 10000;
}



