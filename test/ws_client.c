#include <ws_types.h>
#include <ws_std.h>
#include <ws_inet.h>
#include <ws_socket.h>
#include <errno.h>
#include <time.h>


void testudp();
void testtcp();

int main(int argc, char** argv)
{
    
    time_t                      lastsec, current;
    int                         count;
#ifdef _WIN32
    setvbuf(stdout, 0, _IONBF, 0);
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }

#else
    setbuf(stdout, 0);
#endif

    lastsec = time(0);
    count = 0;
    while (1) {
        
        testtcp();
       // testudp();
        current = time(0);
        if (current == lastsec) {
            count+=2;
        }
        else {
            printf("current qps:%d\n", count);
            count = 0;
            lastsec = current;
        }

    }
    return 0;
}


void testudp()
{
    ws_socket_t                        s;
    int                                 flag;
    struct sockaddr_in                  addr;
    int                                 error;
    ws_itf_header_t                   *itf;
    int                                 size;
    char                                buffer[256];

    s = ws_udp_socket_create();

    if (s <= 0) {
        printf("socket create failed");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.116.128");
    addr.sin_port = htons(80);

    //flag = connect(s, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    //if (flag < 0) {
    //     error = errno;
    //     printf("connect failed!, errno:%d", error);
    // }

    itf = buffer;
    itf->cmd = 0;
    itf->seq = 0;
    itf->size = sizeof(ws_itf_header_t);
    size = sendto(s, buffer, itf->size, 0, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    //printf("send to size:%d\n", size);
    close(s);
}


void testtcp()
{
    ws_socket_t                        s;
    int                                 flag;
    struct sockaddr_in                  addr;
    int                                 error;
    ws_itf_header_t                   *itf;
    int                                 size;
    char                                buffer[256];

    s = ws_tcp_socket_create();

    if (s <= 0) {
        printf("socket create failed");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.116.128");
    addr.sin_port = htons(8080);

    flag = connect(s, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (flag < 0) {
         error = errno;
         printf("connect failed!, errno:%d\n", error);
         return;
     }

    itf = buffer;
    itf->cmd = 0;
    itf->seq = 0;
    itf->size = sizeof(ws_itf_header_t);
    size = send(s, buffer, itf->size, 0);
   // printf("send to size:%d\n", size);
    close(s);

   // while (1) {
  //      Sleep(100);
  //  }
}
