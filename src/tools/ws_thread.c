//
//  ws_thread.c
//  whatsserverz
//
//  Created by peter chain on 13-6-8.
//  Copyright (c) 2013年 peter chain. All rights reserved.
//

#include <ws_types.h>
#include <ws_std.h>
#include <ws_thread.h>

#define _TIMESPEC_DEFINED
#include <pthread.h>


typedef struct ws_thread_s ws_thread_t;



struct ws_thread_s
{
   // pthread_t                    tid;
    ws_thread_task_func         func;
    void                        *data;
    ws_int32_t                         *quit;
    
};

void* ws_thread_func( void  *data );


ws_int32_t ws_thread_regist( ws_thread_task_func func, void  *data, ws_int32_t *quit )
{
    pthread_t               tid;
    ws_thread_t            *thread;

    thread = malloc(sizeof(ws_thread_t));
    if (!thread) {
        return YMZ_ERROR;
    }

    thread->data = data;
    thread->func = func;
    thread->quit = quit;

    pthread_create(&tid, 0, ws_thread_func, thread);
	return 0;

}

void* ws_thread_func( void  *data )
{
    ws_thread_t               *thread;
    ws_int32_t                        *quit;
    void                       *fdata;
    ws_thread_task_func        func;
    
    if( !data ){
        return 0;
    }
    
    thread = data;
    quit = thread->quit;
    func = thread->func;
    fdata = thread->data;
    
    while( !*quit ){
        func( fdata );
    }
    
    return 0;
    
}



ws_int32_t ws_thread_mutex_create( ws_thread_mutex_t** mutex )
{
	

	return YMZ_THREAD_OK;

}

ws_int32_t ws_thread_mutex_lock( ws_thread_mutex_t* mutex )
{
	

	return YMZ_THREAD_OK;
}

ws_int32_t ws_thread_mutex_unlock( ws_thread_mutex_t* mutex )
{


	return YMZ_THREAD_OK;
}

ws_int32_t ws_thread_mutex_free( ws_thread_mutex_t* mutex )
{
	return 0;
}


ws_int32_t ws_thread_rwm_create( ws_thread_rwm_t** rwm )
{
	
	return YMZ_THREAD_OK;

}

ws_int32_t ws_thread_rwm_rlock( ws_thread_rwm_t* rwm )
{

	return YMZ_THREAD_OK;
}

ws_int32_t ws_thread_rwm_wlock( ws_thread_rwm_t* rwm )
{
	

	return YMZ_THREAD_OK;
}

ws_int32_t ws_thread_rwm_unlock( ws_thread_rwm_t* rwm )
{
	

	return YMZ_THREAD_OK;

}

ws_int32_t ws_thread_rwm_free( ws_thread_rwm_t* rwm )
{
	
	return YMZ_THREAD_OK;
}
