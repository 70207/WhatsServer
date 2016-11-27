#include <stdio.h>
#include <signal.h>

#include <setjmp.h>
#include <errno.h>

#include <ws_std.h>
#include <ws_inet.h>

#include <ws_mem.h>
#include <ws_chain.h>
#include <ws_event.h>
#include <ws_conn.h>
#include <ws_cycle.h>
#include <ws_log.h>
#include <ws_process.h>





#include <ws_conn_icmp.h>


ws_int32_t ws_conn_icmp_socket_create()
{
    ws_socket_t             s;
    struct protoent         *protocol;

    if ((protocol = getprotobyname("icmp")) == NULL)
    {
        ws_log_print_error("getprotobyname error");
        return 0;
    }
    /*����ʹ��ICMP��ԭʼ�׽���,�����׽���ֻ��root��������*/
    if ((s = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
    {
        ws_log_print_error("create raw socket error");
        return 0;
    }

    return s;
}


unsigned short cal_chksum(unsigned short *addr, ws_int32_t len)
{
    ws_int32_t           nleft, sum;
    unsigned short      *w;
    unsigned short       answer;

    nleft = len;
    sum = 0;
    w = addr;
    answer = 0;

    /*��ICMP��ͷ������������2�ֽ�Ϊ��λ�ۼ�����*/
    while (nleft>1)
    {
        sum += *w++;
        nleft -= 2;
    }
    /*��ICMP��ͷΪ�������ֽڣ���ʣ�����һ�ֽڡ������һ���ֽ���Ϊһ��2�ֽ����ݵĸ��ֽڣ����2�ֽ����ݵĵ��ֽ�Ϊ0�������ۼ�*/
    if (nleft == 1)
    {
        *(ws_uchar_t *)(&answer) = *(ws_uchar_t *)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}






ws_int32_t ws_conn_send_icmp_echo(ws_socket_t s, ws_conn_icmp_t  *t)
{
#ifdef __linux__
    ws_int32_t               i, packsize;
    struct icmp             *icmp;
    ws_int8_t                buffer[4096];
    struct sockaddr_in       addr;

    icmp = buffer;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = t->seq_id;
    icmp->icmp_id = ws_process_id;
    packsize = 8 + t->data_len;
    memcpy(icmp->icmp_data, t->data, t->data_len);
    icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize); /*У���㷨*/


    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = t->ip;
    addr.sin_port = 0;
    
    if (sendto(s, buffer, packsize, 0,&addr, sizeof(addr))<0)
    {
        ws_log_print_warn("send icmp echo error:%d", errno);
        return YMZ_ERROR;
    }


    ws_log_print_info("send icmp echo to %s", inet_ntoa(addr.sin_addr));
#endif
    return YMZ_OK;
}


ws_int32_t ws_conn_recv_icmp(ws_int8_t *packet, ws_int32_t p_size, ws_conn_icmp_t  *t)
{
#ifdef __linux__
    ws_int32_t          i, iphdrlen;
    struct              ip *ip;
    struct              icmp *icmp;
    struct              timeval *tvsend;
    double              rtt;
    ws_int32_t          size;

    ip = packet;
    iphdrlen = ip->ip_hl << 2;    /*��ip��ͷ����,��ip��ͷ�ĳ��ȱ�־��4*/
    icmp = (struct icmp *)(packet + iphdrlen);  /*Խ��ip��ͷ,ָ��ICMP��ͷ*/
    p_size -= iphdrlen;            /*ICMP��ͷ��ICMP���ݱ����ܳ���*/
    if (p_size<8)                /*С��ICMP��ͷ�����򲻺���*/
    {
        ws_log_print_warn("ICMP packets\'s length is less than 8");
        return YMZ_ERROR;
    }
    /*ȷ�������յ����������ĵ�ICMP�Ļ�Ӧ*/
    if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == ws_process_id))
    {
        t->data = icmp->icmp_data;
       
        ws_log_print_info("recv icmp echo %d byte icmp_seq=%u ttl=%d rtt=%.3f ms\n",
            p_size,
            icmp->icmp_seq,
            ip->ip_ttl,
            rtt);
        return YMZ_OK;
    }
    
#endif
    return YMZ_ERROR;
}
