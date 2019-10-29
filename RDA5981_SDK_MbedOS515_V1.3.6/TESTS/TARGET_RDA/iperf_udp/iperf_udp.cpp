#include "mbed.h"
#include "rtos.h"
#include "WiFiStackInterface.h"
#include "NetworkStack.h"
#include "TCPSocket.h"
#include "console.h"
#include "at.h"
#include "cmsis_os.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/ip.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/dns.h"
#include "lwip/opt.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "iperf_udp.h"

#define IPERF_PORT                      5001

#define BUFSIZE                         1500

enum iperf_status
{
    E_WAITING = 0,
    E_CONNECTED,
    E_CLOSED
};

struct iperf_state {
    enum iperf_status status;
    struct client_hdr chdr;
    uint32_t flags;
};

uint8_t buffer[BUFSIZE];
struct iperf_state iperf;

struct iperf_stats stats;

static ip_addr_t udp_client_ip;
static uint32_t udp_client_port;
static uint32_t test_time;
static uint32_t test_tx;
extern unsigned int os_time;
ip_addr_t remote_addr;
ip_addr_t mult_remote_addr;

static uint32_t port = 12345;

u32_t time1 = 0, total_len = 0, start_time;
extern unsigned int os_time;
static int recv = 0;
static int iperf_udp_recv(struct netconn *conn)
{
    struct UDP_datagram *pkt;
    struct server_hdr *hdr;
    struct netbuf *nbuf = 0;
    struct netbuf *nbuf2 = 0;
    char *buf;
    char *buf2;
    u16_t buflen;
    int32_t datagramID;
    uint32_t done = 0;
    uint32_t started = 0;

    /* Read as much data as possible from the server. */
    while (!done && ERR_OK == netconn_recv(conn, &nbuf)) {

    if (!recv) {
        printf(" Interval       Transfer        Bandwidth \r\n");
        recv = 1;
        start_time = os_time;
        time1 = os_time;
    }
        /* Read packet iperf data. */
        netbuf_data(nbuf, (void**)&buf, &buflen);
        pkt = (UDP_datagram *)buf;
        datagramID = ntohl(pkt->id);

        if (datagramID >= 0) {
            /* Test is running. */
            if (started == 0) {
                /* First packet received, print IP address and initialize stats. */
                ip_addr_t *addr;
                addr = netbuf_fromaddr(nbuf);
                udp_client_port = netbuf_fromport(nbuf);
                udp_client_ip = *addr;
                stats.udp_rx_start_sec = ntohl(pkt->tv_sec);
                stats.udp_rx_start_usec = ntohl(pkt->tv_usec);
                started = 1;
                test_tx = 0;
            }
            /* Update stats. */
            if (stats.udp_rx_seq != (uint32_t) datagramID) {
                stats.udp_rx_lost += (uint32_t) datagramID - stats.udp_rx_seq;
                stats.udp_rx_seq = datagramID + 1;
                stats.udp_rx_outorder += 1;
            }
            else {
                stats.udp_rx_total_pkt += 1;
                stats.udp_rx_total_size += buflen;
                stats.udp_rx_seq += 1;
            }
            total_len += buflen;
            if (os_time - time1 > 1000 && recv) {
                printf(" [%d - %d] sec    %5.1f KBytes   %5.2f Mb/s\r\n",
                (time1 - start_time) / 1000, (os_time - start_time)/ 1000,    total_len * 1000.0 / 1024 / (os_time - time1),
                total_len * 8000.0 / 1024 / (os_time - time1) / 1000);
                time1 = os_time;
                total_len = 0;
            }
        }
        else {
            /* UDP test is now over. */
            if (started) {
                printf(" [0 - %d] sec    %5.2f MBytes   %5.2f Mb/s\r\n",
                (os_time - start_time)/ 1000,    stats.udp_rx_total_size * 1.0 / 1024 / 1000 ,
                stats.udp_rx_total_size * 8000.0 / 1024 / (os_time - start_time) / 1000);
                   recv = 0;
                pkt->id = htonl(datagramID);
                stats.udp_rx_end_sec = ntohl(pkt->tv_sec);
                stats.udp_rx_end_usec = ntohl(pkt->tv_usec);
                test_time = stats.udp_rx_end_sec - stats.udp_rx_start_sec;

                /* Copy packet and send report back. */
                nbuf2 = netbuf_new();
                buf2 = (char *)netbuf_alloc(nbuf2, buflen);
                memcpy(buf2, buf, buflen);
                netbuf_delete(nbuf);
                nbuf = nbuf2;
                pkt = (UDP_datagram *)buf2;
                hdr = (struct server_hdr *)(pkt + 1);
                if (ntohl(hdr->flags) & HEADER_VERSION1) {
                    test_tx = 1;
                }
                hdr->flags        = htonl(HEADER_VERSION1);
                hdr->total_len1   = htonl(0);
                hdr->total_len2   = htonl(stats.udp_rx_total_size);
                hdr->stop_sec     = htonl(stats.udp_rx_end_sec - stats.udp_rx_start_sec);
                hdr->stop_usec    = htonl(stats.udp_rx_end_usec - stats.udp_rx_start_usec);
                hdr->error_cnt    = htonl(stats.udp_rx_lost);
                hdr->outorder_cnt = htonl(stats.udp_rx_outorder);
                hdr->datagrams    = htonl(stats.udp_rx_seq);
                hdr->jitter1      = htonl(0);
                hdr->jitter2      = htonl(0);

                /* Send report to client. */
                netconn_sendto(conn, nbuf2, &udp_client_ip, udp_client_port);
                wait_ms(1);
                netconn_sendto(conn, nbuf2, &udp_client_ip, udp_client_port);
                done = 1;
            }
        }
        /* Free input resource. */
        netbuf_delete(nbuf);
    }
    return done;
}

static void iperf_udp_send(struct netconn *conn)
{
    struct UDP_datagram *pkt;
    struct server_hdr *hdr;
    struct netbuf *nbuf = 0;
    char *buf;
    int32_t datagramID = 0;
    uint32_t start_time = 0;
    uint32_t timeout = 3;
    uint32_t i = 0;

    /* Reset packet header content. */
    pkt = (UDP_datagram *)buffer;
    memset(pkt, 0, sizeof(struct UDP_datagram));
    hdr = (struct server_hdr *)(pkt + 1);
    memset(hdr, 0, sizeof(struct server_hdr));
    hdr->flags        = htonl(HEADER_VERSION1);
    hdr->total_len1   = htonl(0);
    hdr->total_len2   = htonl(IPERF_WIFI_UDP_BUFFER_SIZE);

    start_time = os_time;
    test_time = test_time * 1000; /* Convert to ticks. */

    while (1) {
        /* Send test packet with incremented ID. */
        if (os_time - start_time < test_time) {
            nbuf = netbuf_new();
            buf = (char *)netbuf_alloc(nbuf, IPERF_WIFI_UDP_BUFFER_SIZE);
            memcpy(buf, buffer, IPERF_WIFI_UDP_BUFFER_SIZE);
            pkt->id = ntohl(datagramID++);
            netconn_sendto(conn, nbuf, &remote_addr, IPERF_PORT);
            netbuf_delete(nbuf);
        }
        /* Send test end with neg ID 100 times. */
        else {
            //printf("end \r\n");
            pkt->id = ntohl(-datagramID);
            while (1) {
                nbuf = netbuf_new();
                buf = (char *)netbuf_alloc(nbuf, IPERF_WIFI_UDP_BUFFER_SIZE);
                memcpy(buf, buffer, IPERF_WIFI_UDP_BUFFER_SIZE);
                int8_t ret = netconn_sendto(conn, nbuf, &remote_addr, IPERF_PORT);
                netbuf_delete(nbuf);
                if(ret == ERR_OK || i > 100) {
                	i = 0;
                	break;
                }
               i++;
            }
            break;
        }
    }

    while (1) {
        conn->recv_timeout = 2000;
        int8_t ret = netconn_recv(conn, &nbuf);
        //printf("ret is %d \r\n", ret);
        if (ERR_OK == ret || i > timeout) {
            netbuf_delete(nbuf);
            break;
        }
        i++;
    }
}

static void lwip_udp_mult_send(struct netconn *conn)
{
    struct UDP_datagram *pkt;
    struct server_hdr *hdr;
    struct netbuf *nbuf = 0;
    char *buf;
    int32_t datagramID = 0;
    uint32_t start_time = 0;

    /* Reset packet header content. */
    pkt = (UDP_datagram *)buffer;
    memset(pkt, 0, sizeof(struct UDP_datagram));
    hdr = (struct server_hdr *)(pkt + 1);
    memset(hdr, 0, sizeof(struct server_hdr));
    hdr->flags          = htonl(HEADER_VERSION1);
    hdr->total_len1   = htonl(0);
    hdr->total_len2   = htonl(IPERF_WIFI_UDP_BUFFER_SIZE);

    start_time = os_time;
    test_time = test_time * 1000; /* Convert to ticks. */

    while (1) {
        /* Send test packet with incremented ID. */
        if (os_time - start_time < test_time) {
            nbuf = netbuf_new();
            buf = (char *)netbuf_alloc(nbuf, IPERF_WIFI_UDP_BUFFER_SIZE);
            memcpy(buf, buffer, IPERF_WIFI_UDP_BUFFER_SIZE);
            pkt->id = ntohl(datagramID++);
            netconn_sendto(conn, nbuf, &mult_remote_addr, IPERF_PORT);
            netbuf_delete(nbuf);
        }
        /* Send test end with neg ID 20 times. */
        else {
            pkt->id = ntohl(-datagramID);
            for (uint32_t i = 0; i < 20; ++i) {
                nbuf = netbuf_new();
                buf = (char *)netbuf_alloc(nbuf, IPERF_WIFI_UDP_BUFFER_SIZE);
                memcpy(buf, buffer, IPERF_WIFI_UDP_BUFFER_SIZE);
                netconn_sendto(conn, nbuf, &mult_remote_addr, IPERF_PORT);
                netbuf_delete(nbuf);
            }
            break;
        }
        wait_ms(10);
    }
}

int lwiperf_start_udp_client(const char *address, int timeout)
{
    struct netconn *udp_socket;
     ip4addr_aton(address, &remote_addr);
     test_time = timeout;
    if ((udp_socket = netconn_new(NETCONN_UDP)) == NULL) {
        printf("iperf_udp_task: could not create UDP socket!\n");
        while (1);
    }

    if (netconn_bind(udp_socket, NULL, port++) != ERR_OK) {
        printf("iperf_udp_task: could not bind TCP socket!\n");
        while (1);
    }
    netconn_connect(udp_socket, &remote_addr, IPERF_PORT);
    iperf_udp_send(udp_socket);
    RESP_OK();
    netconn_delete(udp_socket);
    return 0;
}

void lwiperf_start_mult_client(const char *address, int timeout)
{
    struct netconn *udp_socket;
    ip4addr_aton("225.0.0.1", &mult_remote_addr);
    test_time = timeout;
    if ((udp_socket = netconn_new(NETCONN_UDP)) == NULL) {
        printf("iperf_udp_task: could not create UDP socket!\n");
        while (1);
    }

    if (netconn_bind(udp_socket, &mult_remote_addr, port++) != ERR_OK) {
        printf("iperf_udp_task: could not bind TCP socket!\n");
        while (1);
    }

    if (netconn_join_leave_group(udp_socket, &mult_remote_addr, IP_ADDR_ANY, NETCONN_JOIN)) {
        printf("iperf_udp_task: could not join mult group!\n");
        while (1);
    }
    netconn_connect(udp_socket, &mult_remote_addr, IPERF_PORT);
    lwip_udp_mult_send(udp_socket);
    //printf("client send done \r\n");
    RESP_OK();
    netconn_delete(udp_socket);
}

void lwiperf_start_mult_server_default(const char *address)
{
        struct netconn *udp_socket;
        int ret;
        ip_addr_t mult_addr;
        ip4addr_aton(address, &mult_addr);

        /* Create server socket. */
        if ((udp_socket = netconn_new(NETCONN_UDP)) == NULL) {
            printf("iperf_udp_task: could not create UDP socket!\n");
            while (1);
        }
        if (netconn_bind(udp_socket, &mult_addr, IPERF_PORT) != ERR_OK) {
            printf("iperf_udp_task: could not bind TCP socket!\n");
            while (1);
        }
    
        if (netconn_join_leave_group(udp_socket,&mult_addr, IP_ADDR_ANY, NETCONN_JOIN)) {
            printf("iperf_udp_task: could not join mult group!\n");
            while (1);
        }
        
        while (1) {
            /* Clear UDP stats. */
            memset(&stats, 0, sizeof(struct iperf_stats));
    
            /* Receive as long as server is sending. */
            ret = iperf_udp_recv(udp_socket);
            
            if (ret) {
                break;
            }
        }
        RESP_OK();
        netconn_delete(udp_socket);
}

void lwiperf_start_udp_server_default()
{
    struct netconn *udp_socket;
    int ret;
    /* Create server socket. */
    if ((udp_socket = netconn_new(NETCONN_UDP)) == NULL) {
        printf("iperf_udp_task: could not create UDP socket!\n");
        while (1);
    }
    if (netconn_bind(udp_socket, NULL, IPERF_PORT) != ERR_OK) {
        printf("iperf_udp_task: could not bind TCP socket!\n");
        while (1);
    }
    
    while (1) {
        /* Clear UDP stats. */
        memset(&stats, 0, sizeof(struct iperf_stats));

        /* Receive as long as server is sending. */
        ret = iperf_udp_recv(udp_socket);
        
        if (ret) {
             break;
        }
    }
    RESP_OK();
    netconn_delete(udp_socket);
}
