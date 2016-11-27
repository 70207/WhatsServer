#ifndef  _WS_INET_H__
#define _WS_INET_H__

#ifndef _WIN32
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#else
#include <WinSock2.h>
#include <WS2tcpip.h>
#define close(s) closesocket(s)
#endif

#endif