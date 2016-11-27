#ifndef _YMZ_MEM_H_
#define _YMZ_MEM_H_


#define YMZ_MAX_ALLOC_FROM_POOL		4096
#define YMZ_MEM_POOL_BLOCK_SIZE		16*1024
#define YMZ_MEM_POOL_BLOCK_NUM		4

#define YMZ_MEM_OK                   0
#define YMZ_MEM_ERROR                1

#include <ws_types.h>


typedef struct ws_mem_buf_s             ws_mem_buf_t;
typedef struct ws_mem_pool_s            ws_mem_pool_t;
typedef struct ws_mem_pool_large_s      ws_mem_pool_large_t;

typedef struct ws_mem_fix_buf_s         ws_mem_fix_buf_t;
typedef struct ws_mem_fix_pool_s        ws_mem_fix_pool_t;

struct ws_mem_buf_s				    
{
	ws_int32_t                         size;
	ws_int32_t                         pos;			
	ws_int32_t                         hd;
	char                       *data;
	ws_mem_pool_t              *pool;
	ws_int32_t							isneedfree;
};

typedef struct ws_mem_pool_data_s			    
{
	ws_uchar_t*				end;			    
	ws_uchar_t*				last;			   
	ws_mem_pool_t*				next;			    
}ws_mem_pool_data_t;

struct ws_mem_pool_large_s					   
{
	void*					data;			    
	long long               size;               
	ws_mem_pool_large_t*	next;			   
};

struct ws_mem_pool_s						    
{
	ws_mem_pool_data_t		pd;				    
    ws_mem_pool_t*          next;
    ws_int32_t						blocks;
    ws_int32_t						size;
    ws_int32_t						reserve;
	ws_int32_t						max;
	ws_mem_pool_t*			current;		    
	ws_mem_pool_large_t*	large;			   
};

struct ws_mem_fix_pool_s
{
	ws_int32_t				    num;
	ws_int32_t				    size;
	ws_int32_t				    used;

	ws_mem_fix_buf_t*   free_bufs;
};

struct ws_mem_fix_buf_s
{
	ws_char_t*               data;
	ws_int32_t		            pos;
	ws_int32_t                 size;

	ws_mem_fix_buf_t*   next;
};

#define ws_mem_raw_malloc(size)		malloc(size)
#define ws_mem_type_malloc(type)	malloc(sizeof(type))
#define ws_mem_raw_free(data)		free(data)
#define ws_mem_set(data,size)       memset(data, 0, size)
#define ws_mem_typeset(data, type)  memset(data, 0, sizeof(type))

ws_mem_pool_t*	                    ws_mem_pool_create(ws_int32_t size , ws_int32_t block_num );               
ws_int32_t                                 ws_mem_pool_free( ws_mem_pool_t* pool);                                   

ws_mem_buf_t*	                    ws_mem_buf_alloc( ws_mem_pool_t* pool, ws_int32_t size );                
ws_int32_t                                 ws_mem_buf_free( ws_mem_buf_t* buf );                                     

ws_int32_t                                 ws_mem_copy( ws_mem_buf_t* src, ws_mem_buf_t* dst );
ws_int32_t                                 ws_mem_copy_raw( ws_uchar_t* buffer, ws_int32_t len, ws_mem_buf_t* dst );

ws_mem_buf_t*		                ws_mem_buf_malloc_need_free(ws_int32_t size);
void*                               ws_mem_malloc(ws_int32_t size);
void                                ws_mem_free(void* data);



ws_mem_fix_pool_t*                  ws_mem_fix_pool_create( ws_int32_t size, ws_int32_t num );
ws_int32_t                                 ws_mem_fix_pool_free( ws_mem_fix_pool_t* pool );
ws_mem_fix_buf_t*                   ws_mem_fix_buf_get( ws_mem_fix_pool_t*   pool );
ws_int32_t                                 ws_mem_fix_buf_put( ws_mem_fix_pool_t* pool, ws_mem_fix_buf_t* buf );

#endif



