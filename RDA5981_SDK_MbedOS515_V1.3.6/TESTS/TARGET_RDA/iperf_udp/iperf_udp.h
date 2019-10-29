#ifndef IPERF_UDP
#define IPERF_UDP

#define IPERF_WIFI_SERVER_PORT                (5001)
#define IPERF_WIFI_TCP_BUFFER_SIZE            (1400)
#define IPERF_WIFI_UDP_BUFFER_SIZE            (1460)

/** iPerf Settings */
#define HEADER_VERSION1                        0x80000000
#define RUN_NOW                                0x00000001

struct UDP_datagram {
    int32_t id;
    uint32_t tv_sec;
    uint32_t tv_usec;
};

struct client_hdr {
    /*
     * flags is a bitmap for different options
     * the most significant bits are for determining
     * which information is available. So 1.7 uses
     * 0x80000000 and the next time information is added
     * the 1.7 bit will be set and 0x40000000 will be
     * set signifying additional information. If no
     * information bits are set then the header is ignored.
     * The lowest order diferentiates between dualtest and
     * tradeoff modes, wheither the speaker needs to start
     * immediately or after the audience finishes.
     */
    int32_t flags;
    int32_t numThreads;
    int32_t mPort;
    int32_t bufferlen;
    int32_t mWinBand;
    int32_t mAmount;
};

struct server_hdr {
    /*
     * flags is a bitmap for different options
     * the most significant bits are for determining
     * which information is available. So 1.7 uses
     * 0x80000000 and the next time information is added
     * the 1.7 bit will be set and 0x40000000 will be
     * set signifying additional information. If no 
     * information bits are set then the header is ignored.
     */
    int32_t flags;
    int32_t total_len1;
    int32_t total_len2;
    int32_t stop_sec;
    int32_t stop_usec;
    int32_t error_cnt;
    int32_t outorder_cnt;
    int32_t datagrams;
    int32_t jitter1;
    int32_t jitter2;
};

struct iperf_stats {
    uint32_t udp_rx_total_size;
    uint32_t udp_rx_total_pkt;
    uint32_t udp_rx_seq;
    uint32_t udp_rx_lost;
    uint32_t udp_rx_outorder;
    uint32_t udp_rx_start_sec;
    uint32_t udp_rx_start_usec;
    uint32_t udp_rx_end_sec;
    uint32_t udp_rx_end_usec;
};

int lwiperf_start_udp_client(const char *address, int timeout);
void lwiperf_start_udp_server_default();
void lwiperf_start_mult_client(const char *address, int timeout);
void lwiperf_start_mult_server_default(const char *address);

#endif // IPERF_UDP
