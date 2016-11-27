#ifndef _WS_TYPES_H__
#define _WS_TYPES_H__





typedef  void                                   ws_void_t;
typedef  char                                   ws_char_t;
typedef   unsigned char                         ws_uchar_t;
typedef   char									ws_int8_t;
typedef   unsigned char							ws_uint8_t;
typedef   short									ws_int16_t;
typedef   unsigned short						ws_uint16_t;
typedef  double                                 ws_double_t;



typedef  int  									ws_int32_t;
typedef  unsigned int 							ws_uint32_t;

typedef  long long								ws_int64_t;
typedef  unsigned long long						ws_uint64_t;
typedef  long									ws_ptr_t;
typedef  char									ws_boolean_t;
typedef  unsigned long long						ws_msec_t;
typedef  unsigned long long						ws_sec_t;


typedef void							        ws_void;
typedef unsigned char					        u_char;
typedef unsigned long long				        ws_udid_t;


typedef  long                                   ws_socket_t;
typedef  unsigned int                           ws_ip_t;
typedef  int                                    ws_port_t;




#define YMZ_BOOL_TRUE							1
#define YMZ_BOOL_FALSE							0


#define YMZ_OK									0
#define YMZ_DONE									20
#define YMZ_ERROR								100
#define YMZ_SYSTEM_ERROR							200
#define YMZ_AGAIN								300
#define YMZ_NO_CUR_USER							400


typedef struct ws_str_s
{
    ws_uint32_t			size;
    void*   			    data;
}ws_str_t;

typedef struct ws_buf_s
{
    ws_uint32_t			size;
    void*    			    data;
}ws_buf_t;

#ifndef _WIN32
#define offsetof(type, member) (size_t)&(((type*)0)->member)
#define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#else
#define container_of(ptr, type, member)  (type *)( (char *)ptr - offsetof(type,member))
#endif
#pragma pack(1)

typedef struct ws_itf_header_s  ws_itf_header_t;

struct ws_itf_header_s
{
    ws_int32_t    					size;
    ws_int32_t  					seq;
    ws_int32_t 		    		    cmd;
};

#pragma pack()

#define ws_thread_volatile   volatile


#ifdef WIN32
#define inline __inline
#endif



typedef union ws_bts_u ws_bts_t;


union ws_bts_u{
    char                         buf;
    int                          ih;
};

typedef struct ws_list_head_s ws_list_head_t;

struct ws_list_head_s {
	ws_list_head_t *next, *prev;
};


#ifdef WIN32
#define ws_inline __inline
#else
#define ws_inline inline
#endif



#define ws_abs(value)       (((value) >= 0) ? (value) : - (value))
#define ws_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define ws_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

static inline void YMZ_INIT_LIST_HEAD(ws_list_head_t *list)
{
	list->next = list;
	list->prev = list;
}

#define YMZ_LIST_HEAD_INIT(name) { &(name), &(name) }

#define YMZ_LIST_HEAD(name) \
	ws_list_head_t name = YMZ_LIST_HEAD_INIT(name)

static inline void __ws_list_add(ws_list_head_t *new,
								ws_list_head_t *prev,
								ws_list_head_t *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void ws_list_add(ws_list_head_t *new, ws_list_head_t *head)
{
	__ws_list_add(new, head, head->next);
}

static inline void ws_list_add_tail(ws_list_head_t *new, ws_list_head_t *head)
{
    __ws_list_add(new, head->prev, head);
}

#define ws_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

static inline void __ws_list_del(ws_list_head_t * prev, ws_list_head_t * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void __ws_list_del_entry(ws_list_head_t *entry)
{
	__ws_list_del(entry->prev, entry->next);
}

static inline void ws_list_del(ws_list_head_t *entry)
{
	__ws_list_del(entry->prev, entry->next);
	entry->next = entry;
	entry->prev = entry;
}

static inline ws_list_head_t* ws_list_del_next(ws_list_head_t *entry)
{
    ws_list_head_t             *next;
    if (entry->next == entry) {
        return 0;
    }

    next = entry->next;
    ws_list_del(next);

    return next;
}

static inline ws_int32_t ws_list_iterator(ws_list_head_t *entry, ws_list_head_t **iterator)
{
    *iterator = 0;
    if (entry->next == entry) {
        return YMZ_ERROR;
    }

    *iterator = entry->next;

    ws_list_del(*iterator);
    ws_list_add_tail(*iterator, entry);

    return YMZ_OK;
}

#endif
