#ifndef _WS_OS_H__
#define _WS_OS_H__

#ifdef  _WIN32
#include <Windows.h>
#endif

ws_int32_t ws_started_check();
void ws_os_single_process();
void ws_os_master_process();
void ws_os_daemon_process();


#endif //_WS_OS_H__
