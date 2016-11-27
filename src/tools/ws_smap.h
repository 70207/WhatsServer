#ifndef _WS_MAP_H__
#define _WS_MAP_H__


#include <ws_mem.h>

typedef struct ws_smap_s ws_smap_t;

typedef struct ws_smap_i_s ws_smap_i_t;
typedef struct ws_smap_ii_s ws_smap_ii_t;

struct ws_smap_i_s
{
    ws_int32_t length;
    ws_int32_t num;
    ws_smap_ii_t* items;
};

struct ws_smap_ii_s
{
    ws_char_t* name;
    ws_char_t* value;
    ws_int32_t next_t;
    ws_smap_i_t*  parent;
    ws_smap_ii_t* prev;
    ws_smap_ii_t* next;
};

struct ws_smap_s
{
    ws_int32_t num;
    ws_mem_pool_t* pool;
    ws_smap_i_t** items;
};

ws_smap_t* ws_smap_alloc(ws_mem_pool_t* pool);
ws_int32_t ws_smap_insert(ws_smap_t* smap, const ws_char_t* name, const ws_char_t* value);
ws_smap_ii_t* ws_smap_get(ws_smap_t* smap, const ws_char_t* name);










#endif