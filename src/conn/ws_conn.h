#ifndef _WS_CONN_H__
#define _WS_CONN_H__

#include <ws_std.h>
#include <ws_inet.h>
#include <ws_event_timer.h>
#include <ws_event.h>
#include <ws_cln_timer.h>
#include <whatsserver.h>

#define YMZ_CONN_ERROR -1
#define YMZ_CONN_OK 0

#define YMZ_CONN_SEND_BUF_SIZE (1024*32)



#define YMZ_CONN_RECV_OK				     0
#define YMZ_CONN_RECV_ERROR				 1
#define YMZ_CONN_RECV_AGAIN				 2
#define YMZ_CONN_CLIENT_CLOSE			 3

#define YMZ_CONN_RECV_SHOULD_HANDLE		 4
#define YMZ_CONN_RECV_NO_HANDLE			 5


#define YMZ_CONN_RECV_PER                 1024

#define YMZ_CONN_RECV_MAX_SIZE		     2048


typedef struct ws_listen_s ws_listen_t;
typedef struct ws_conn_s ws_conn_t;


typedef ws_int32_t (*ws_conn_handle)( ws_conn_t* c);
typedef ws_int32_t(*ws_listen_tcp_handle)(ws_conn_t* c);
typedef ws_int32_t(*ws_listen_udp_handle)(ws_listen_t *l, ws_itf_header_t* itf, ws_int32_t size, ws_ip_t ip, ws_port_t port);
typedef ws_int32_t (*ws_conn_udp_handle)( ws_conn_t* c,  void *data, void *addr );





struct ws_listen_s
{
    ws_rbtree_node_t          node;
	ws_socket_t               s;
	ws_int32_t                        port;
	unsigned long long         ip;
    ws_int32_t                        type;   //1 tcp 2 udp 

	ws_int32_t						   conn_life_time;

    ws_listen_tcp_handle      tcp_handle;
    ws_listen_udp_handle      udp_handle;

    ws_server_tcp_t           tcp_server;
    ws_server_udp_t           udp_server;

    ws_int32_t                server_index;

    void                        *buffer;

    ws_conn_t                 *c;

};




#define YMZ_CONN_TYPE_TCP         1
#define YMZ_CONN_TYPE_UDP         2
#define YMZ_CONN_TYPE_HTTP        4
#define YMZ_CONN_TYPE_WEBSOCKET   8

struct ws_conn_s{
	ws_socket_t               s;
	ws_int32_t				   index;
	struct sockaddr           *sockaddr;
	socklen_t                  socklen;

	time_t                     start_time;
	time_t                     expire_time;

	void                      *pool;
	void                      *recv_buf;
	void                      *send_buf;

	char					  *send_buf2;

	ws_int32_t						   send_pos;
	ws_int32_t						   send_wpos;

	ws_int32_t						   read_buffer_size;
	ws_int32_t						   write_buffer_size;
	ws_int32_t						   handle_pos;
	ws_int32_t						   recv_pos;
	ws_int32_t						   recv_mark;
	ws_int32_t						   max_recv;
	ws_int32_t						   last_finish;

	void					            *send_chain;

	ws_int32_t						   chain_num;
	
	void                                *rev;
	void                                *wev;

	void                                *next;

    void                                *listening;
	ws_int32_t                        is_listen;
    
    void                                *data;
	long					            user_data;
    long                                user_data2;

    ws_int32_t                        type;

	ws_int32_t						   pending_state;
    
    void                                *hostname;

	ws_msec_t 				            life_time;
	ws_int32_t						   close;

	ws_int32_t						   port;

    ws_int32_t                        linger;

	ws_clean_up_t			            *clean_up;

    ws_server_tcp_t                    tcp_server;

    ws_int32_t                        server_index;
};

ws_int32_t ws_listen_cycle_init();
ws_int32_t ws_listen_close();

ws_int32_t ws_conn_cycle_init();
ws_conn_t* ws_conn_get( ws_socket_t s );
ws_int32_t ws_conn_free( ws_conn_t *c );

ws_conn_t* ws_conn_get2(ws_int32_t index);

ws_int32_t ws_socket_nonblock( ws_socket_t s );
ws_int32_t ws_socket_reuse( ws_socket_t s );

ws_int32_t ws_conn_close( ws_conn_t *c );

ws_conn_t* ws_conn_udp_get( ws_int32_t ip, ws_int32_t port );
ws_int32_t ws_conn_udp_close( ws_conn_t   *c );
ws_int32_t ws_conn_udp_send( ws_conn_t   *c, char  *buf, ws_int32_t len,  ws_int32_t ip, ws_int32_t port );

void ws_conn_add_clean_up( ws_conn_t* c,  ws_clean_up_t* cln );
void ws_conn_del_clean_up( ws_conn_t* c,  ws_clean_up_t* cln );

void ws_conn_cork( ws_socket_t s );
void ws_conn_ncork( ws_socket_t s );
void ws_conn_nodelay( ws_socket_t s );
ws_int32_t  ws_conn_socket_udp_create();

ws_int32_t ws_listen_tcp_listen_create(ws_ip_t  ip, ws_int32_t port, ws_listen_t** pplistener);
ws_int32_t ws_listen_udp_listen_create(ws_ip_t  ip, ws_int32_t port, ws_listen_t** pplistener);



#endif
