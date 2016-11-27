
/*
 * The memory management module
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ws_types.h>
#include <ws_log.h>
#include <ws_mem.h>


                                             
ws_int32_t       ws_mem_palloc_block(ws_mem_buf_t* pmembuf);                             
ws_int32_t       ws_mem_palloc_large(ws_mem_buf_t* pmembuf);                             
ws_void*       ws_mem_pool_malloc(ws_mem_pool_t* pool, ws_int32_t size);             

ws_void        ws_mem_debug_free(ws_void* data, ws_int32_t size, long long* ul_size);	
ws_void*       ws_mem_debug_malloc(ws_int32_t size, long long* ul_size);					
ws_void        ws_mem_debug_print_pool(ws_mem_pool_t* pool);

long long mem_count = 0;      //used by debug

void* ws_mem_malloc(ws_int32_t size)										//申请堆内存
{
	void* pdate = malloc(size);
	if (!pdate)
	{
		ws_log_sys_error("[MEM]memory malloc failed! size : %d", size);
		return NULL;
	}

    return pdate;
}

void  ws_mem_free(void* data)										//释放堆内存
{
	free(data);
}

ws_int32_t ws_mem_palloc_block(ws_mem_buf_t* pmembuf)
{
	ws_mem_pool_t *pool, *n_pool, *current;
	ws_uint32_t psize;


	if(!pmembuf)
	{
	    ws_log_sys_error("[MEM]pool malloc block failed! pool pointer is NULL");
        return YMZ_MEM_ERROR;
	}


	
    pool = pmembuf->pool;
    n_pool = ws_mem_pool_create( pool->size, pool->blocks );
    if( !n_pool ){
        return YMZ_MEM_ERROR;
    }

    pool->current->pd.next = n_pool;
    pool->current = n_pool;

    n_pool->next = pool->next;
    pool->next = n_pool;

    pmembuf->data = n_pool->pd.last;
    n_pool->pd.last += pmembuf->size;

    return YMZ_MEM_OK;

}



ws_int32_t ws_mem_palloc_large(ws_mem_buf_t* pmembuf)			//申请大内存块
{
	ws_mem_pool_large_t* large;
	void* pdata;
    ws_int32_t size;
    ws_mem_pool_t* pool;

	if(!pmembuf)
	{
	    ws_log_sys_error("[MEM]pool alloc large memory failed! memory buffer pointer is NULL");
	    return YMZ_MEM_ERROR;
	}

    pmembuf->data = 0;
	pdata = (ws_mem_pool_large_t*)ws_mem_malloc(pmembuf->size);
	
	if ( !pdata )
	{
        ws_log_print_error("mem malloc large failed! size:%d", pmembuf->size);
		return YMZ_MEM_ERROR;
	}

    ws_log_sys_info("alloc large size:%d, data:%llu", pmembuf->size, pdata);

    pool = pmembuf->pool;
    size = sizeof( ws_mem_pool_large_t );
    large = ws_mem_pool_malloc( pool, size );
    if (!large) {
        ws_log_sys_error("large mem malloc error! size:%d", size);
        return YMZ_MEM_ERROR;
    }
    large->data = pdata;
    large->size = pmembuf->size;
    large->next = pool->large;
	pool->large = large;
	pmembuf->data = pdata;

    return YMZ_MEM_OK;
}

ws_mem_pool_t* ws_mem_pool_create( ws_int32_t size, ws_int32_t block_num)		//创建内存池
{
    ws_int32_t i;
    ws_int32_t reserve;
    ws_char_t* pdata;
    ws_int32_t psize;
	ws_mem_pool_t* pool, *n_pool;
    ws_mem_pool_data_t* m_p_d;
    ws_int32_t             ssize;

    ssize = size;
	if (size < sizeof(ws_mem_pool_t))
	{
		ws_log_sys_error("[MEM]memory pool create too small! size : %d", size);
		return NULL;
	}

    size += 1024;
    psize = size * block_num;
	#ifndef MEM_DEBUG
	pdata = ws_mem_malloc( size * block_num );
	#else
	pdata = ws_mem_debug_malloc(size, &mem_count);
	#endif

    if (!pdata) {
        ws_log_sys_error("pool memory create failed, no enough memory!");
        return NULL;
    }

    pool = pdata;

	reserve = size - sizeof(ws_mem_pool_t);
	pool->pd.last = pdata + sizeof(ws_mem_pool_t);
	pool->pd.end = pdata + size;
	pool->pd.next = NULL;
	pool->current = pool;
	pool->large = NULL;
    pool->reserve = reserve;
    pool->size = size;
    pool->blocks = block_num;
    pool->next = 0;
    pool->max = ssize;
        //(reserve > YMZ_MAX_ALLOC_FROM_POOL) ? YMZ_MAX_ALLOC_FROM_POOL : reserve;

	//下面代码创建block_num-1个block（内存池数据块）以链表形式pool的后面


    m_p_d = &pool->pd;
    psize = sizeof(ws_mem_pool_t);

	for(i = 1; i < block_num; ++i)
	{
        pdata += size;
        n_pool = pdata;
        n_pool->pd.end = pdata + size;
	    n_pool->pd.next = NULL;
	    n_pool->pd.last = pdata + psize;
        m_p_d->next = n_pool;
        m_p_d = &n_pool->pd;
	}

	//ws_log_access_info("[MEM]memory pool create successed! block size is %d, block number is %d\n", size, block_num);

    #ifdef MEM_PRINT_POOL
    ws_mem_debug_print_pool(pool);
    #endif

	return pool;
}

ws_int32_t ws_mem_pool_free( ws_mem_pool_t* pool)						//释放内存池
{

	ws_mem_pool_t *p, *n;
	ws_mem_pool_large_t *large, *l;

	if (!pool)
	{
	    ws_log_sys_error("[MEM]pool free faild! pool pointer is NULL!");
		return -1;
	}



	//释放申请的大块儿内存
	large = pool->large;
	while(large)
	{
		if (large->data)
		{
            ws_log_sys_info("free large data size:%d, data:%llu", large->size, large->data);
			ws_mem_free(large->data);
			large->data = NULL;
	
		}
		l = large;
		large = l->next;

	}

	pool->large = NULL;

	//释放内存池数据块
	p = pool;
	while(p)
	{
        n = p->next;
		ws_mem_free(p);
		p = n;
	}
    
	return 0;
}

ws_mem_buf_t* ws_mem_buf_alloc( ws_mem_pool_t* pool, ws_int32_t size )	//从内存池申请内存
{
 
    ws_mem_buf_t* pmem_buf;
    ws_mem_pool_t* p;
    ws_uchar_t* data;
    ws_int32_t isize;

	if (!pool)
	{
	    ws_log_sys_error("[MEM]pool memory alloc faild! pool pointer is NULL!");
		return NULL;
	}
    
    isize = sizeof( ws_mem_buf_t );

	pmem_buf = ws_mem_pool_malloc(pool, isize );
	if ( !pmem_buf )
	{
		return NULL;
	}


    pmem_buf->pool = pool;
	pmem_buf->pos = 0;
	pmem_buf->size = size;
	pmem_buf->hd = 0;

    //ws_log_access_debug("[MEM]malloc memory from pool, size is %u\n", size);

	if (size <= pool->max)
	{
		p = pool->current;
		do
		{
            if ( p->pd.end - p->pd.last >= size)
			{
                pmem_buf->data = p->pd.last;
				p->pd.last += size;
				return pmem_buf;
			}
			p = p->pd.next;

		} while (p);

        if (YMZ_MEM_OK != ws_mem_palloc_block(pmem_buf)) {
            return 0;
        }
		return pmem_buf;
	}
	else
	{
        if (YMZ_MEM_OK != ws_mem_palloc_large(pmem_buf)) {
            return 0;
        }
		return pmem_buf;
	}
}

void* ws_mem_pool_malloc(ws_mem_pool_t* pool, ws_int32_t size)
{
    ws_int32_t flag;
	ws_uchar_t* data;
    ws_mem_buf_t pmem_buf;
  
    ws_mem_pool_t* p;
    

    if (NULL == pool || size > pool->max) {
        return NULL;
    }

	pmem_buf.pool = pool;
    pmem_buf.size = size;
    pmem_buf.pos = 0;

   // ws_log_sys_error("[MEM]pool malloc used by yourself! size is %d", size);

   
    p = pool->current;
	do
	{
		data = p->pd.last;
		if (p->pd.end - p->pd.last >= size)
		{
            p->pd.last +=  size;
			return data;
		}
		p = p->pd.next;
	} while (p);

	flag = ws_mem_palloc_block(&pmem_buf);
    if( flag ){
        return 0;
    }

	pmem_buf.isneedfree = 0;
	return pmem_buf.data;
}

ws_int32_t ws_mem_buf_free( ws_mem_buf_t* buf )							//释放内存到内存池中
{
	ws_mem_pool_large_t *large;
	ws_mem_pool_t* pool;

	if (!buf)
	{
	    ws_log_sys_error("[MEM]buf free failed! buf is NULL %llu", buf);
		return -1;
	}

	if (buf->isneedfree){
		free(buf);
	}
    //大内存块即时释放
	

	pool = buf->pool;
	large = pool->large;
	while(large)
	{
		if (large->data == buf->data)
		{
            ws_log_sys_info("free large data size:%d, data:%llu", large->size, large->data);
		    #ifndef MEM_DEBUG
			ws_mem_free(large->data);
			#else
			ws_mem_debug_free(large->data, large->size, &mem_count);
			#endif
			large->data = NULL;


			break;
		}
		large = large->next;
	}

	//内存池的小块儿内存等到内存池释放的时候释放

	return 0;
}

ws_int32_t ws_mem_copy( ws_mem_buf_t* src, ws_mem_buf_t* dst )
{
	ws_int32_t remain;
	if( !src || !dst ){
		return YMZ_MEM_ERROR;
	}

	remain = dst->size - dst->pos;
	if( src->size < remain ){
		remain = src->size;
	}
	if( remain <= 0 ){
		return YMZ_MEM_ERROR;
	}

	memcpy( dst->data + dst->pos, src->data, remain );

	dst->pos += remain;

	return YMZ_MEM_OK;
}

ws_int32_t ws_mem_copy_raw( ws_uchar_t* buffer, ws_int32_t len, ws_mem_buf_t* dst )
{
	ws_int32_t remain;
	if( !buffer || !dst ){
		return YMZ_MEM_ERROR;
	}

	remain = dst->size - dst->pos;
	if( len < remain ){
		remain = len;
	}
	if( remain <= 0 ){
		return YMZ_MEM_ERROR;
	}

	memcpy( dst->data + dst->pos, buffer, remain );

	dst->pos += remain;

	return YMZ_MEM_OK;
}



ws_mem_buf_t*		  ws_mem_buf_malloc_need_free(ws_int32_t size)
{
	ws_int32_t						len;
	void				   *data;
	ws_mem_buf_t	       *buf;

	len = size + sizeof(ws_mem_buf_t);
	data = malloc(len);

	if (!data){
		return 0;
	}

	buf = data;
	
	buf->data = (ws_char_t*)data + sizeof(ws_mem_buf_t);
	buf->size = size;
	buf->hd = 0;
	buf->isneedfree = 1;
	buf->pool = 0;
	buf->pos = 0;
	
	return buf;

}

ws_mem_fix_pool_t* ws_mem_fix_pool_create( ws_int32_t size, ws_int32_t num )
{
	ws_mem_fix_pool_t*			pool;
	ws_mem_fix_buf_t          *buf,*next;
	ws_int32_t						    tsize;
	char					   *data, *data2, *data3;
	ws_int32_t							i;

	tsize = sizeof( ws_mem_fix_pool_t );
	tsize += size * num;
	tsize += num * sizeof( ws_mem_fix_buf_t );

	data = malloc( tsize );

	if( !data ){
		return 0;
	}
	
	memset( data, 0, tsize );
	pool = data;
	data += sizeof( ws_mem_fix_pool_t );
	data2 = data;
	data3 = data2 + sizeof( ws_mem_fix_buf_t ) * num;
	buf = data2;

	for( i = 0; i < num; ++i ){
		buf->size = size;
		buf->data = data3 + i * size;
		++buf;
	}

	buf = data2;

	for( i = 1; i < num; ++i ){
		next = buf + 1;
		buf->next = next;
		buf = next;
	}

	pool->free_bufs = data2;
	pool->num = num;
	pool->size = size;
	

	return pool;

}

ws_int32_t ws_mem_fix_pool_free( ws_mem_fix_pool_t* pool )
{
	free( pool );
    return 0;
}

ws_mem_fix_buf_t* ws_mem_fix_buf_get( ws_mem_fix_pool_t*   pool )
{
	ws_mem_fix_buf_t		*buf;
	if( pool->used >= pool->size ){
		return 0;
	}

	buf = pool->free_bufs;
	pool->free_bufs = buf->next;

	++pool->used;

	return buf;
}

ws_int32_t ws_mem_fix_buf_put( ws_mem_fix_pool_t* pool, ws_mem_fix_buf_t* buf )
{
	buf->next = pool->free_bufs;
	pool->free_bufs = buf;
	
	--pool->used;

	return 0;

}