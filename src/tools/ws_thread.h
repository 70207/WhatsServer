//
//  ws_thread.h
//  whatsserverz
//
//  Created by peter chain on 13-6-8.
//  Copyright (c) 2013年 peter chain. All rights reserved.
//

#ifndef _WS_THREAD_H__
#define _WS_THREAD_H__

#include <ws_types.h>


#define YMZ_THREAD_OK         0
#define YMZ_THREAD_ERROR     -1

typedef void ws_thread_mutex_t;
typedef void ws_thread_rwm_t;
typedef void (*ws_thread_task_func )( void* data );

ws_int32_t ws_thread_regist( ws_thread_task_func func, void  *data, ws_int32_t *quit );


ws_int32_t ws_thread_mutex_create( ws_thread_mutex_t** mutex );
ws_int32_t ws_thread_mutex_lock( ws_thread_mutex_t* mutex );
ws_int32_t ws_thread_mutex_unlock( ws_thread_mutex_t* mutex );
ws_int32_t ws_thread_mutex_free( ws_thread_mutex_t* mutex );


ws_int32_t ws_thread_rwm_create( ws_thread_rwm_t** rwm );
ws_int32_t ws_thread_rwm_rlock( ws_thread_rwm_t* rwm );
ws_int32_t ws_thread_rwm_wlock( ws_thread_rwm_t* rwm );
ws_int32_t ws_thread_rwm_unlock( ws_thread_rwm_t* rwm );
ws_int32_t ws_thread_rwm_free( ws_thread_rwm_t* rwm );


#endif
