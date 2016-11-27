#include <ws_std.h>
#include <ws_inet.h>


#include <errno.h>
#include <time.h>

#include <ws_types.h>
#include <ws_time.h>
#include <ws_mem.h>
#include <ws_event.h>
#include <ws_event_timer.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_rbtree.h>
#include <ws_log.h>



#if defined(_WIN32) || defined(__APPLE__)

ws_rbtree_t                 ws_event_select_tree;
ws_rbtree_node_t            ws_event_select_sentinel;
YMZ_LIST_HEAD(ws_event_select_list);

typedef struct ws_event_select_s
{
    ws_rbtree_node_t               node;
    ws_list_head_t                 list;
    ws_int32_t                     type;
    ws_conn_t                     *conn;
    ws_socket_t                    s;

}ws_event_select_t;

ws_int32_t ws_event_select_process_init()
{
    ws_rbtree_init(&ws_event_select_tree, &ws_event_select_sentinel, ws_rbtree_insert_value);
    return 0;

}

ws_int32_t ws_event_select_close()
{
    ws_list_head_t                 *list;
    ws_event_select_t              *event;

    list = ws_event_select_list.next;
    while (list != &ws_event_select_list) {
        event = container_of(list, ws_event_select_t, list);
        list = list->next;
        free(event);
    }

    YMZ_INIT_LIST_HEAD(&ws_event_select_list);

    ws_rbtree_init(&ws_event_select_tree, &ws_event_select_sentinel, ws_rbtree_insert_value);
    return 0;
}

#define YMZ_EVENT_SELECT_FD_RESERVE 2048
void ws_event_select_process()
{
    ws_list_head_t                 *list;
    ws_event_select_t              *event;
    fd_set                           rs, ws, es;
    ws_int32_t                      max_fd, flag;
    ws_int32_t                      event_c, i;
    ws_socket_t                     fd;
    ws_conn_t                       *c;
    ws_event_t                      *rev, *wev;
    ws_socket_t                     fd_temp[YMZ_EVENT_SELECT_FD_RESERVE];
    struct timeval                   time_out;
    ws_rbtree_key_t                 key;
    ws_rbtree_node_t               *node;

    FD_ZERO(&rs);
    FD_ZERO(&ws);
    FD_ZERO(&es);

    max_fd = 0;
    event_c = 0;
    ws_list_for_each(list, &ws_event_select_list) {
        event = container_of(list, ws_event_select_t, list);
        if (event_c >= YMZ_EVENT_SELECT_FD_RESERVE) {
            ws_log_print_warn("select event too much!");
            break;
        }
        if (event->type & YMZ_EVENT_TYPE_READ) {
            FD_SET(event->s, &rs);
        }
        if (event->type & YMZ_EVENT_TYPE_WRITE) {
            FD_SET(event->s, &ws);
        }
        FD_SET(event->s, &es);
        if (event->s > max_fd) {
            max_fd = event->s;
        }
        fd_temp[event_c++] = event->s;
    }

    time_out.tv_sec = 0;
    time_out.tv_usec = 5000;

    flag = select(max_fd + 1, &rs, &ws, &es, &time_out);
    switch (flag) {
    case -1:
        ws_log_print_warn("select error");
        break;
    case 0:
        return;
    }
    
    list = ws_event_select_list.next;
    for (i = 0; i < event_c; ++i){
   
        fd = fd_temp[i];
        key.v = fd;
        key.v2 = 0;

        if (YMZ_OK != ws_rbtree_get(&ws_event_select_tree, key, &ws_event_select_sentinel, &node)) {
            continue;
        }

        event = container_of(node, ws_event_select_t, node);
        c = event->conn;

        if (FD_ISSET(fd, &rs)) {
            rev = c->rev;
            rev->handle(rev);
        }
        
        if (FD_ISSET(fd, &ws)) {
            wev = c->wev;
            wev->handle(wev);
        }

        if (FD_ISSET(fd, &es)) {
            rev = c->rev;
            rev->timedout = 1;
            rev->handle(rev);
        }
    }


}


ws_int32_t ws_event_select_add(ws_event_t* ev, ws_int32_t type)
{
    ws_rbtree_node_t                    *node;
    ws_rbtree_key_t                      key;
    ws_event_select_t                   *event;


    key.v = ev->fd;
    key.v2 = 0;

    if (YMZ_OK == ws_rbtree_get(&ws_event_select_tree, key, &ws_event_select_sentinel, &node)) {
        event = container_of(node, ws_event_select_t, node);
        event->conn = ev->data;
        event->type |= type;
        event->s = key.v;
        return YMZ_OK;
    }

    event = malloc(sizeof(ws_event_select_t));
    if (!event) {
        ws_log_print_warn("event malloc error!");
        return YMZ_ERROR;
    }

    memset(event, 0, sizeof(ws_event_select_t));
    event->conn = ev->data;
    event->type = type;
    event->s = key.v;
    YMZ_INIT_LIST_HEAD(&event->list);
    node = &event->node;

    node->key = key;
    ws_rbtree_insert(&ws_event_select_tree, node);
    ws_list_add(&event->list, &ws_event_select_list);

    return 0;
}

ws_int32_t ws_event_select_add_conn(void* conn)
{
    ws_rbtree_node_t                    *node;
    ws_rbtree_key_t                      key;
    ws_event_select_t                   *event;
    ws_conn_t                           *c;


    c = conn;
    key.v = c->s;
    key.v2 = 0;

    if (YMZ_OK == ws_rbtree_get(&ws_event_select_tree, key, &ws_event_select_sentinel, &node)) {
        event = container_of(node, ws_event_select_t, node);
        event->conn = c;
        event->s = key.v;
        event->type = YMZ_EVENT_TYPE_READ | YMZ_EVENT_TYPE_WRITE;
        return YMZ_OK;
    }

    event = malloc(sizeof(ws_event_select_t));
    if (!event) {
        ws_log_print_warn("event malloc error!");
        return YMZ_ERROR;
    }

    memset(event, 0, sizeof(ws_event_select_t));
    event->conn = c;
    event->s = key.v;
    event->type = YMZ_EVENT_TYPE_READ | YMZ_EVENT_TYPE_WRITE;

    node = &event->node;

    node->key = key;
    YMZ_INIT_LIST_HEAD(&event->list);
    ws_rbtree_insert(&ws_event_select_tree, node);
    ws_list_add(&event->list, &ws_event_select_list);

    return 0;
}

ws_int32_t ws_event_select_del(ws_event_t* ev, ws_int32_t flags)
{
    ws_rbtree_node_t                    *node;
    ws_rbtree_key_t                      key;
    ws_event_select_t                   *event;
    


    key.v = ev->fd;
    key.v2 = 0;

    if (YMZ_OK != ws_rbtree_get(&ws_event_select_tree, key, &ws_event_select_sentinel, &node)) {
        return YMZ_OK;
    }

    event = container_of(node, ws_event_select_t, node);

    if (flags & YMZ_EVENT_CLOSE) {
        ws_list_del(&event->list);
        ws_rbtree_delete(&ws_event_select_tree, node);
        free(event);
    }
    
    if (flags & YMZ_EVENT_TYPE_READ) {
        event->type &= ~YMZ_EVENT_TYPE_READ;
    }
    
    if (flags &YMZ_EVENT_TYPE_WRITE) {
        event->type &= ~YMZ_EVENT_TYPE_WRITE;
    }

    if (event->type == 0) {
        ws_list_del(&event->list);
        ws_rbtree_delete(&ws_event_select_tree, node);
        free(event);
    }
    
    return YMZ_OK;


}

ws_int32_t ws_event_select_del_conn(void* conn)
{
    ws_rbtree_node_t                    *node;
    ws_rbtree_key_t                      key;
    ws_event_select_t                   *event;
    ws_conn_t                           *c;


    c = conn;
    key.v = c->s;
    key.v2 = 0;

    if (YMZ_OK != ws_rbtree_get(&ws_event_select_tree, key, &ws_event_select_sentinel, &node)) {
        return YMZ_OK;
    }

    event = container_of(node, ws_event_select_t, node);

  
    ws_list_del(&event->list);
    ws_rbtree_delete(&ws_event_select_tree, node);
    free(event);
    return YMZ_OK;


}


#endif