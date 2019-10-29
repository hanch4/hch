#include "mbed.h"
#include "rtos.h"
#include "WiFiStackInterface.h"
#include "NetworkStack.h"
#include "TCPSocket.h"
#include "console.h"
#include "at.h"
#include "cmsis_os.h"
#include "iperf_udp.h"

WiFiStackInterface wifi;
static char ssid[32+1];
static char pw[64+1];
static char conn_flag = 0;

int do_wsconn( cmd_tbl_t *cmd, int argc, char *argv[])
{
    char *ssid_t, *pw_t;
    const char *ip_addr;
    int ret;

    if (argc < 1) {
        RESP_ERROR(ERROR_ARG);
        return 0;
    }

    if (*argv[1] == '?') {
        if (conn_flag == 1) {
            wifi.update_rssi();
            RESP_OK_EQU("ssid:%s, RSSI:%ddb, ip:%s\r\n", wifi.get_ssid(), wifi.get_rssi(), wifi.get_ip_address());
        } else {
            RESP_OK_EQU("ssid:, RSSI:0db, ip:0.0.0.0\r\n");
        }
        return 0;
    }

    if (conn_flag == 1) {
        RESP_ERROR(ERROR_ABORT);
        RDA_AT_PRINT("error! Has been connected!");
        return 0;
    }

    memset(ssid, 0, sizeof(ssid));
    memset(pw, 0, sizeof(pw));

    if (argc > 1)
        memcpy(ssid, argv[1], strlen(argv[1]));

    if (argc > 2)
        memcpy(pw, argv[2], strlen(argv[2]));

    if (strlen(ssid) != 0)
        ssid_t = ssid;
    else
        ssid_t = NULL;

    if (strlen(pw) != 0)
        pw_t = pw;
    else
        pw_t = NULL;

    RDA_AT_PRINT("ssid %s pw %s\r\n", ssid_t, pw_t);

    ret = wifi.connect(ssid_t, pw_t, NULL, NSAPI_SECURITY_NONE, 0);
    if (ret != 0) {
        RESP_ERROR(ERROR_FAILE);
        return 0;
    }
    ip_addr = wifi.get_ip_address();
    if (ip_addr) {
        printf("Connect %s success! Client IP Address is %s\r\n", ssid_t, ip_addr);
        conn_flag = 1;
        RESP_OK();
    } else {
        RDA_AT_PRINT("No Client IP Address\r\n");
        RESP_ERROR(ERROR_FAILE);
    }

    return 0;
}

int do_nstart(cmd_tbl_t *cmd, int argc, char *argv[]) {

    if (conn_flag == 0) {
        printf("Please connect ap!\r\n");
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }
    lwiperf_start_udp_server_default();

    return 0;
}

int do_mstart(cmd_tbl_t *cmd, int argc, char *argv[]) {
    char SERVER_ADDR[20];
    ip4_addr_t groupaddr;

     if (argc < 1) {
        printf("Please set server ip!\r\n");
        RESP_ERROR(ERROR_ARG);
        return 0;
    }

    if (conn_flag == 0) {
        printf("Please connect ap!\r\n");
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }

    memcpy(SERVER_ADDR, argv[1], strlen(argv[1]));
    ip4addr_aton(SERVER_ADDR, &groupaddr);
    if (!ip4_addr_ismulticast(&groupaddr)) {
        printf("Error! %s is non-multicast address \r\n", SERVER_ADDR);
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }
    lwiperf_start_mult_server_default(SERVER_ADDR);
    return 0;
}

int do_mcstart(cmd_tbl_t *cmd, int argc, char *argv[])
{
    char SERVER_ADDR[20];
    int timeout;
    ip4_addr_t groupaddr;

     if (argc < 1) {
        printf("Please set server ip!\r\n");
        RESP_ERROR(ERROR_ARG);
        return 0;
    }
    if (conn_flag == 0) {
        printf("Please connect ap!\r\n");
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }
    memcpy(SERVER_ADDR, argv[1], strlen(argv[1]));
    ip4addr_aton(SERVER_ADDR, &groupaddr);
    if (!ip4_addr_ismulticast(&groupaddr)) {
        printf("Error! %s is non-multicast address \r\n", SERVER_ADDR);
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }

    timeout = atoi(argv[2]);
    if (timeout < 0) {
        RESP_ERROR(ERROR_ARG);
        return 0;
    }
    lwiperf_start_mult_client(SERVER_ADDR , timeout);

    return 0;
}


int do_cstart(cmd_tbl_t *cmd, int argc, char *argv[])
{
    char SERVER_ADDR[20];
    int timeout;

     if (argc < 1) {
        printf("Please set server ip!\r\n");
        RESP_ERROR(ERROR_ARG);
        return 0;
    }
    if (conn_flag == 0) {
        printf("Please connect ap!\r\n");
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }
    memcpy(SERVER_ADDR, argv[1], strlen(argv[1]));
    timeout = atoi(argv[2]);
    if (timeout < 0) {
        RESP_ERROR(ERROR_ARG);
        return 0;
    }
    lwiperf_start_udp_client(SERVER_ADDR , timeout);

    return 0;

}

int do_disconn( cmd_tbl_t *cmd, int argc, char *argv[]) {
    if (conn_flag == 0) {
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }

    wifi.disconnect();
    while (wifi.get_ip_address() != NULL)
        Thread::wait(20);
    conn_flag = 0;
    RESP_OK();
    return 0;
}

void add_cmd() {
    int i, j;
    cmd_tbl_t cmd_list[] = {
        /*Basic CMD*/
        {
            "AT+WSCONN",        3,   do_wsconn,
            "AT+WSCONN          - start wifi connect"
        },
        {
            "AT+WSDISCONN",     1,   do_disconn,
            "AT+WSDISCONN       - disconnect"
        },
        /*NET CMD*/
        {
            "AT+NSSTART",        1,   do_nstart,
            "AT+NSSTART          - start udp server iperf speed test"
        },
        
        {
            "AT+NCSTART",        3,   do_cstart,
            "AT+NCSTART          - start udp client iperf speed test"
        },
        {
            "AT+MSSTART",         2,   do_mstart,
            "AT+NSSTART          - start multicast server speed test"
        },
        
        {
            "AT+MCSTART",         3,   do_mcstart,
            "AT+NCSTART          - start multicast client speed test"
        },

    };
    j = sizeof(cmd_list)/sizeof(cmd_tbl_t);
    for (i = 0; i < j; i++) {
        if (0 != console_cmd_add(&cmd_list[i])) {
            RDA_AT_PRINT("Add cmd failed\r\n");
        }
    }
}

void start_at(void) {
    console_init();
    add_cmd();
}

int main() {
    printf("udp iperf test begin\r\n");
    start_at();
}

