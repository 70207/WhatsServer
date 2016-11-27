#include <ws_std.h>
#include <ws_inet.h>

#ifdef __linux__
#include <sys/epoll.h>
#endif

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





#ifdef __linux__

#define ws_log_event_warn   ws_log_access_warn
#define ws_log_event_error  ws_log_access_error
#define ws_log_event_info   ws_log_access_info
#define ws_log_event_debug  ws_log_access_debug

static ws_int32_t ep;
static struct epoll_event* ep_events;
static ws_int32_t ep_n;
static ws_int32_t statis_process = 0;
static ws_sec_t statis_last = 0;





ws_int32_t ws_event_epoll_process_init()
{
    ws_mem_buf_t*			buf;
    ws_mem_pool_t*			pool;
    ws_cycle_t*			cycle;
    ws_int32_t						size;
    struct epoll_event*		events;
    ws_conn_t*				c;
    ws_event_t*			rev;

    cycle = ws_cycle;
    if (!cycle) {
        ws_log_sys_warn("event init error for cycle is null");
        return YMZ_ERROR;
    }


    pool = cycle->pool;

    ep_n = cycle->event_reserve;

    size = sizeof(struct epoll_event);
    buf = ws_mem_buf_alloc(pool, size * ep_n);
    if (!buf) {
        ws_log_sys_warn("event init error for mem is null");
        return YMZ_ERROR;
    }

    events = buf->data;
    ep_events = events;

    ep = epoll_create(cycle->event_reserve);
    if (ep < 0) {
        ws_log_sys_warn("event init error for epoll create error");
        return YMZ_ERROR;
    }


    return YMZ_OK;

}

ws_int32_t ws_event_epoll_close()
{
    close(ep);
    return YMZ_CYCLE_OK;
}

void ws_event_epoll_process()
{
    ws_int32_t				flag;
    ws_int32_t				e_num;
    ws_int32_t				i;
    ws_int32_t				revents;
    ws_conn_t*		c;
    ws_event_t*	rev;
    ws_event_t*	wev;

    ws_sec_t		current_sec;
    ws_msec_t		current_msec;
    ws_int32_t				d, h, m, s;


    //time_t timer = ws_event_timer_get();
    //if( timer <= 0 || timer > 500 ){

    ws_int32_t	timer = 500;

    //}

    e_num = epoll_wait(ep, ep_events, ep_n, timer);
    if (e_num < 0) {
        switch (errno) {
        case EBADF:
        case EINVAL:
            ws_log_error("event process error for ep is wrong");
            return;
        }
    }

    if (e_num <= 0) {
        return;
    }

    ws_time_update();
    current_sec = ws_current_sec;

    if (current_sec > statis_last) {
        /*d = current_sec % 86400;
        h = d / 3600;
        m = ( d % 3600 ) / 60;
        s = d % 60;
        printf( "process:%d, time:%02d:%02d:%02d handle %d processes\n", ws_cycle->process_id, h, m, s, statis_process );*/
        statis_process = 0;
        statis_last = current_sec;
    }

    statis_process += e_num;

    for (i = 0; i < e_num; ++i) {
        c = ep_events[i].data.u64;

        if (c->s == -1) {
            ws_log_error("event process socket -1\n");
            continue;
        }

        revents = ep_events[i].events;
        if (revents & (EPOLLERR | EPOLLHUP)) {
            ws_log_print_debug("epoll event error, revents:%d", revents);
            ws_conn_close(c);
            continue;
        }

        rev = c->rev;

        if ((revents & EPOLLIN) && rev->active) {
            if (rev->handle) {
                rev->handle(rev);
            }
        }

        wev = c->wev;
        if ((revents & EPOLLOUT) && wev->active) {
            if (wev->handle) {
                wev->handle(wev);
            }
        }

    }

    return;

}


ws_int32_t ws_event_epoll_add(ws_event_t* ev, ws_int32_t type)
{
    ws_conn_t* c;
    struct epoll_event ee;
    ws_event_t* e;
    ws_int32_t prev;
    ws_int32_t etype;
    ws_int32_t do_type;

    if (!ev) {
        return YMZ_EVENT_ERROR;
    }

    if (ev->active) {
        return YMZ_EVENT_OK;
    }


    etype = 0;
    c = ev->data;

    if (type & YMZ_EVENT_TYPE_READ)
    {
        e = c->wev;
        prev = EPOLLOUT;
        etype |= EPOLLIN;
    }
    else {
        e = c->rev;
        prev = EPOLLIN;
        etype |= EPOLLOUT;
    }

    if (type & YMZ_EVENT_TYPE_ET)
    {
        etype |= EPOLLET;
    }

    if (e->active) {
        do_type = EPOLL_CTL_MOD;
        etype |= prev;
    }
    else {
        do_type = EPOLL_CTL_ADD;
    }

    ev->type = type;
    ee.events = etype;

    ee.data.u64 = c;
    epoll_ctl(ep, do_type, ev->fd, &ee);

    ev->active = 1;

    return YMZ_EVENT_OK;
}

ws_int32_t ws_event_epoll_add_conn(void* conn)
{
    ws_conn_t*			  c;
    struct epoll_event	  ee;
    ws_int32_t					  etype;
    ws_event_t*		  rev;
    ws_event_t*		  wev;


    c = conn;

    rev = c->rev;
    wev = c->wev;

    rev->active = 1;
    wev->active = 1;


    etype = EPOLLIN | EPOLLOUT | EPOLLET;

    rev->type = YMZ_EVENT_TYPE_READ;
    wev->type = YMZ_EVENT_TYPE_WRITE;

    ee.events = etype;
    ee.data.u64 = c;

    epoll_ctl(ep, EPOLL_CTL_ADD, rev->fd, &ee);


    return YMZ_EVENT_OK;
}

ws_int32_t ws_event_epoll_del(ws_event_t* ev, ws_int32_t flags)
{
    ws_conn_t* c;
    struct epoll_event ee;
    ws_event_t* e;
    ws_int32_t prev;
    ws_int32_t do_type;
    ws_int32_t type;

    if (!ev) {
        return YMZ_EVENT_ERROR;
    }

    if (flags & YMZ_EVENT_CLOSE) {
        ev->active = 0;
        return YMZ_EVENT_OK;
    }

    if (!ev->active) {
        return YMZ_EVENT_ERROR;
    }

    c = ev->data;
    type = ev->type;

    if (type & YMZ_EVENT_TYPE_READ)
    {
        e = c->wev;
        prev = EPOLLOUT | EPOLLET;
    }
    else {
        e = c->rev;
        prev = EPOLLIN | EPOLLET;
    }

    if (e->active) {
        do_type = EPOLL_CTL_MOD;
        ee.events = prev;

    }
    else {
        do_type = EPOLL_CTL_DEL;
        ee.events = 0;

    }

    ev->type = 0;
    ee.data.u64 = c;

    epoll_ctl(ep, do_type, ev->fd, &ee);

    ev->active = 0;

    return YMZ_EVENT_OK;

}

ws_int32_t ws_event_epoll_del_conn(void* conn)
{
    ws_conn_t*				c;
    struct epoll_event		ee;
    ws_event_t*			rev, *wev;


    c = conn;

    rev = c->rev;
    wev = c->wev;


    ee.events = 0;

    epoll_ctl(ep, EPOLL_CTL_DEL, c->s, &ee);

    rev->active = 0;
    wev->active = 0;

    return YMZ_EVENT_OK;
}


#endif