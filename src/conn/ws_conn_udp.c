//
//  ws_conn_udp.c
//  whatsserverz
//
//  Created by peter chain on 13-6-20.
//  Copyright (c) 2013年 peter chain. All rights reserved.
//

#include <ws_std.h>
#include <ws_inet.h>
#include <ws_conn.h>



ws_int32_t ws_conn_udp_send( ws_conn_t   *c, char  *buf, ws_int32_t len,  ws_int32_t ip, ws_int32_t port )
{
    ws_int32_t                     s;
    ws_int32_t                     flag;
    ws_int32_t                     size;
    
    struct sockaddr_in      addr;
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = port;
    
    s = c->s;
    size = sizeof( struct sockaddr_in );
    
    flag = sendto( s,  buf, size,  0, ( struct sockaddr *)&addr, size );
    return flag;
}
