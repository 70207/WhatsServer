//
//  yumei_time.h
//  whatsserver
//
//  Created by peter chain on 13-6-8.
//  Copyright (c) 2013年 peter chain. All rights reserved.
//

#ifndef _WS_TIME_H__
#define _WS_TIME_H__

#include <ws_types.h>

void ws_time_init();
void ws_time_update();

ws_int64_t	ws_time_times_get(ws_msec_t timer);
ws_int64_t	ws_time_times_get_static(ws_msec_t timer);
ws_int64_t	ws_current_times_get();
ws_int64_t  ws_time_mtime_get(ws_int64_t times);



extern ws_msec_t				ws_current_usec;
extern ws_msec_t				ws_current_msec;
extern ws_sec_t				    ws_current_sec;
extern ws_msec_t                ws_current_year_month;


extern volatile ws_str_t		ws_cached_err_log_time;
extern volatile ws_str_t		ws_cached_err_log_utime;
extern volatile ws_str_t        ws_cached_time_format_yyyyMMddhhmmss;
extern volatile ws_str_t        ws_cached_time_format_yyyy_MMdd;
extern volatile ws_str_t        ws_cached_time_format_yyyy_MM_dd_hh_mm_ss;

#endif
