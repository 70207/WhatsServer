#include <ws_std.h>

#include <ws_types.h>
#include <ws_mem.h>
#include <ws_smap.h>

#define YMZ_SMAP_RESERVE 32

ws_int32_t ws_smap_insert_item(ws_smap_i_t* i_item, const ws_char_t* name, const ws_char_t* value, ws_int32_t lenN, ws_int32_t lenV);
ws_smap_ii_t* ws_smap_get_item(ws_smap_i_t* i_item, const ws_char_t* name);

ws_smap_t* ws_smap_alloc(ws_mem_pool_t* pool)
{
    ws_smap_t* smap;
    ws_smap_i_t** items;
    ws_smap_i_t* item;
    ws_int32_t size;
    ws_int32_t i;

    size = sizeof(ws_smap_t);
    smap = (ws_smap_t*)ws_mem_buf_alloc(pool, size);
    memset(smap, 0, size);

    size = sizeof(ws_smap_i_t*) * YMZ_SMAP_RESERVE;
    items = (ws_smap_i_t**)ws_mem_buf_alloc(pool, size);
    size = sizeof(ws_smap_i_t);

    for (i = 0; i < YMZ_SMAP_RESERVE; ++i)
    {
        item = (ws_smap_i_t*)ws_mem_buf_alloc(pool, size);
        item->length = i;
        item->num = 0;
        items[i] = item;
    }

    smap->items = items;
    return smap;
}

ws_int32_t ws_smap_insert(ws_smap_t* smap, const ws_char_t* name, const ws_char_t* value)
{
    ws_mem_pool_t* pool;
    ws_smap_i_t** i_items;
    ws_smap_i_t*  i_item;
    ws_int32_t size;
    ws_int32_t lenN;
    ws_int32_t lenV;

    if (!smap || !name || !value)
    {
        return 0;
    }

    pool = smap->pool;
    i_items = smap->items;
    lenN = strlen(name);
    lenV = strlen(value);
    i_item = i_items[lenN - 1];
    smap->num++;

    return ws_smap_insert_item(i_item, name, value, lenN, lenV);

}

ws_smap_ii_t* ws_smap_get(ws_smap_t* smap, const ws_char_t* name)
{
    ws_int32_t lenN;
    ws_smap_i_t* i_item;

    if (!smap || !name)
    {
        return 0;
    }

    lenN = strlen(name);
    if (lenN >= YMZ_SMAP_RESERVE) {
        return 0;
    }

    i_item = smap->items[lenN - 1];
    if (!i_item->num)
    {
        return 0;
    }


    return ws_smap_get_item(i_item, name);
}


ws_int32_t ws_smap_insert_item(ws_smap_i_t* i_item, const ws_char_t* name, const ws_char_t* value, ws_int32_t lenN, ws_int32_t lenV)
{
    ws_smap_ii_t* ii_item;
    ws_smap_ii_t* ii_item_prev;
    ws_smap_ii_t* ii_item_next;
    ws_smap_ii_t* ii_item_new;
    ws_mem_pool_t* pool;
    const ws_char_t* cur;
    ws_int32_t next_t;
    ws_int32_t size;
    size = sizeof(ws_smap_ii_t);


    //ii_item_new = (ws_smap_ii_t*)ws_mem_buf_alloc(pool, size);
    //ii_item_new->name = name;
    //ii_item_new->value = value;
    //ii_item_new->parent = i_item;
    //ii_item_new->prev = ii_item_new->next = 0;
    //ii_item_new->next_t = 0;

    //i_item->num++;

    //if (!i_item->items)
    //{
    //    i_item->items = ii_item_new;
    //    return 0;
    //}

    //ii_item = i_item->items;
    //ii_item_prev = 0;
    //next_t = 0;
    //while (ii_item)
    //{
    //    if (ii_item_prev)
    //    {
    //        if (ii_item_prev->next_t > next_t)
    //        {
    //            if (!ii_item_next)
    //            {
    //                ii_item->next = ii_item_new;
    //                ii_item_new->prev = ii_item;
    //                break;
    //            }

    //            ii_item = ii_item->next;
    //            ii_item_prev = ii_item;
    //            ii_item_next = ii_item->next;
    //            continue;
    //        }
    //        else if (ii_item_prev->next_t < next_t)
    //        {
    //            ii_item_prev->next = ii_item;
    //            ii_item->prev = ii_item_prev;
    //            ii_item_new->name = ii_item;
    //            ii_item->prev = ii_item_new;
    //            ii_item_new->next_t = ii_item_prev->next_t;
    //            break;
    //        }
    //    }

    //    for (;next_t < lenN; ++next_t)
    //    {
    //        if (ii_item->name[next_t] > name[next_t])
    //        {
    //            ii_item_new->next_t = next_t;
    //            ii_item_new->next = ii_item;
    //            if (!ii_item->prev)
    //            {
    //                i_item->items = ii_item_new;
    //            }
    //            else
    //            {
    //                ii_item->prev->next = ii_item_new;
    //                ii_item_new->prev = ii_item->prev;
    //            }

    //            ii_item->prev = ii_item_new;
    //        }
    //        else if (ii_item->name[next_t] < name[next_t])
    //        {
    //            if (!ii_item->next)
    //            {
    //                ii_item->next = ii_item_new;
    //                ii_item_new->prev = ii_item;
    //                ii_item->next_t = next_t;
    //            }
    //            else
    //            {
    //                ii_item_prev = ii_item;
    //                ii_item = ii_item->next;
    //                ii_item_next = ii_item->next;
    //                break;
    //            }
    //        }
    //    }
    //    if (next_t == lenN)
    //    {
    //        break;
    //    }
    //}

    return 0;

}


ws_smap_ii_t* ws_smap_get_item(ws_smap_i_t* i_item, const ws_char_t* name)
{
    ws_smap_ii_t* ii_item;
    ws_smap_ii_t* ii_item_prev;
    ws_smap_ii_t* ii_item_next;
    ws_smap_ii_t* ii_item_new;
    ws_mem_pool_t* pool;
    const ws_char_t* cur;
    ws_int32_t next_t;
    ws_int32_t size;

    if (!i_item->items)
    {
        return 0;
    }

    ii_item = i_item->items;
    next_t = 0;

    while (ii_item)
    {
        for (;next_t < i_item->length; ++next_t)
        {
            if (ii_item->name[next_t] > name[next_t])
            {
                return 0;
            }
            else if (ii_item->name[next_t] < name[next_t])
            {
                ii_item_prev = ii_item;
                ii_item = ii_item->next;
                break;
            }
        }

        if (next_t == i_item->length)
        {
            return ii_item;
        }
    }


    return 0;
}
