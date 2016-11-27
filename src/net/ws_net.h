#ifndef _WS_NET_H__
#define _WS_NET_H__

#include <ws_mem.h>
#include <ws_conn.h>
#include <whatsserver.h>


#define YMZ_NET_OK			0
#define YMZ_NET_ERROR		10

#define YMZ_NET_RSP_MAX_SIZE				1024
#define YMZ_NET_TCP_HEADER_SIZE				2
#define YMZ_NET_TCP_SEQID_SIZE				4


typedef struct ws_net_s ws_net_t;


struct ws_net_s
{
	ws_mem_pool_t			*pool;
	ws_conn_t				*conn;

    ws_itf_header_t     	*reh;
	void				        *rsp;

	void					*send_chain;

	ws_event_handle_ptr	 read_event_handler;
	ws_event_handle_ptr	 write_event_handler;

	ws_char_t*					 read_buffer;
	ws_int32_t						 read_buffer_size;
	ws_int32_t						 seq_id;


	void					*data;
};




ws_int32_t ws_net_module_init();
ws_int32_t ws_net_cache_module_init();

ws_int32_t ws_net_send( ws_net_t* net );

ws_int32_t ws_net_close( ws_net_t* net );

ws_int32_t ws_net_tcp_server_add(ws_server_tcp_t *server);
ws_int32_t ws_net_udp_server_add(ws_server_udp_t *server);

ws_int32_t ws_net_tcp_server_add2(ws_listen_t  *listen);
ws_int32_t ws_net_udp_server_add2(ws_listen_t  *listen);
ws_int32_t ws_net_server_del(ws_int32_t server_index);

//send
ws_int32_t ws_net_tcp_send(ws_int32_t server_index, ws_int32_t conn_index, ws_itf_header_t* itf);
ws_int32_t ws_net_udp_send(ws_int32_t server_index, ws_ip_t ip, ws_port_t port,  ws_itf_header_t* itf);
#endif //_WS_NET_H__
