#include "mbed.h"
#include "rtos.h"
#include "WiFiStackInterface.h"
#include "rda_sys_wrapper.h"
#include "rda5981_sniffer.h"

typedef struct rda_sema {
    osSemaphoreId    sema_id;
    osSemaphoreDef_t def;
    uint32_t         data[2];
} rda_sema_t;

/* Frame Type and Subtype Codes (6-bit) */
enum TYPESUBTYPE_T{
    ASSOC_REQ             = 0x00,
    ASSOC_RSP             = 0x10,
    REASSOC_REQ           = 0x20,
    REASSOC_RSP           = 0x30,
    PROBE_REQ             = 0x40,
    PROBE_RSP             = 0x50,
    BEACON                = 0x80,
    ATIM                  = 0x90,
    DISASOC               = 0xA0,
    AUTH                  = 0xB0,
    DEAUTH                = 0xC0,
    ACTION                = 0xD0,
    PS_POLL               = 0xA4,
    RTS                   = 0xB4,
    CTS                   = 0xC4,
    ACK                   = 0xD4,
    CFEND                 = 0xE4,
    CFEND_ACK             = 0xF4,
    DATA                  = 0x08,
    DATA_ACK              = 0x18,
    DATA_POLL             = 0x28,
    DATA_POLL_ACK         = 0x38,
    NULL_FRAME            = 0x48,
    CFACK                 = 0x58,
    CFPOLL                = 0x68,
    CFPOLL_ACK            = 0x78,
    QOS_DATA              = 0x88,
    QOS_DATA_ACK          = 0x98,
    QOS_DATA_POLL         = 0xA8,
    QOS_DATA_POLL_ACK     = 0xB8,
    QOS_NULL_FRAME        = 0xC8,
    QOS_CFPOLL            = 0xE8,
    QOS_CFPOLL_ACK        = 0xF8,
    BLOCKACK_REQ          = 0x84,
    BLOCKACK              = 0x94,
};

WiFiStackInterface wifi;
rda_sema_t sw_mode = {NULL,};;
rda5981_scan_result scan_result[64] = {{0,},};

static char hex_to_number(char a)
{
    if (a >= '0' && a <= '9')
        return a - '0';
    else if (a >= 'a' && a <= 'z')
        return a - 'a' + 0xa;
    else if (a >= 'A' && a <= 'Z')
        return a - 'A' + 0xa;

    printf("Invalid char: %02X\r\n", a);
    return '\0';
}

int my_smartconfig_handler(unsigned short data_len, void *data)
{
    u16 seq_tmp;
    char type;
    const char *typc_c;
    char *frame = (char *)data;
    char *da = NULL;
    if (data==NULL) {
        printf("ldpc:%d\n", data_len);
    } else {
        signed char rssi;
        unsigned char chnl;
        da = frame + 4;
        seq_tmp = (frame[22] | (frame[23]<<8))>>4;
        type = frame[0] & 0xfc;
        rssi = *((signed char *)(frame + data_len));
        chnl = *((unsigned char *)(frame + data_len + 1));
        switch(type) {
            case BEACON:
                typc_c = "BEACON";
                break;
            case QOS_DATA:
            case DATA:
                if ((frame[1]&0x3) == 0x1)
                    typc_c = "TODS_DATA";
                else
                    typc_c = "FROMDS_DATA";
                break;
            case PROBE_REQ:
                typc_c = "PROBE_REQ";
                break;
            case PROBE_RSP:
                typc_c = "PROBE_RSP";
                break;
            case AUTH:
                typc_c = "AUTH";
                break;
            case ASSOC_REQ:
                typc_c = "ASSOC_REQ";
                break;
            case ASSOC_RSP:
                typc_c = "ASSOC_RSP";
                break;
            default:
                typc_c = "unknown";
                printf("unknown type:0x%x\n", type);
        }

        printf("%d, %d, %s, seq:%d, len:%d\n", chnl, rssi, typc_c, seq_tmp, data_len);
        if (type == ASSOC_REQ ||
            type == AUTH ||
            type == PROBE_REQ) {
            int i, flag = 0;
            const char *ap_mac_char = wifi.get_mac_address_ap();
            char ap_mac[6];

            for(i = 0; i < 6; ++i) {
                ap_mac[i] = hex_to_number(ap_mac_char[i * 3]) << 4 | hex_to_number(ap_mac_char[i * 3 + 1]);
            }

            //printf("%02x:%02x:%02x:%02x:%02x:%02x",
                    //ap_mac[0], ap_mac[1], ap_mac[2], ap_mac[3], ap_mac[4], ap_mac[5]);

            if(memcmp(da, ap_mac, 6) == 0) {
                printf("parse MacAddr, type %02X\r\n", type);
                flag = 1;
            } else if(type == PROBE_REQ) {
                unsigned char eid, ssid_len;
                unsigned char *ssid_str;
                eid = *((unsigned char *)(frame + 24));
                ssid_len = *((unsigned char *)(frame + 25));
                ssid_str = (unsigned char *)(frame + 26);
#if 0
                if((0U == eid) && (0 < ssid_len) && (32 > ssid_len)) {
                    char tmp[32] = {0,};
                    memcpy(tmp, ssid_str, ssid_len);
                    tmp[ssid_len] = '\0';
                    printf("ProbReq2SSID:%s\r\n", tmp);
                }
#endif
                if((0U == eid) && (9 == ssid_len)) {
                    if(memcmp("RRDD-8889", ssid_str, ssid_len) == 0) {
                        printf("parse probe SSID\r\n");
                        flag = 1;
                    }
                }
            }
            if(1 == flag) {
                osStatus ret;
                ret = osSemaphoreRelease(sw_mode.sema_id);
                if(ret) {
                    printf("release sema err\r\n");
                }
            }
        }
    }
    return 0;
}

int main() {
    int ret, cnt = 1;
    unsigned int msg;
    void *main_msgQ = rda_msgQ_create(5);
    int i;
    printf("\r\nStart sniffer_softAP test...\r\n");

    memset(sw_mode.data, 0, sizeof(sw_mode.data));
    sw_mode.def.semaphore = sw_mode.data;
    sw_mode.sema_id = osSemaphoreCreate(&sw_mode.def, 0);
    if(NULL == sw_mode.sema_id) {
        printf("Create sema err: %d\r\n", (int)sw_mode.sema_id);
        osDelay(1);
        return -1;
    }

    wifi.set_msg_queue(main_msgQ);

    printf("Start softAP\r\n");
    ret = wifi.start_ap("RRDD-8889", NULL, 11);
    if(ret) {
        printf("start ap err: %d\r\n", ret);
        return -1;
    }
    osDelay(150);

    printf("Enable sniffer mode\r\n");
    rda5981_enable_sniffer(my_smartconfig_handler);
    rda5981_start_sniffer(6, 1, 1, 1, 0);

    i = 1;
    do {
        int ret;
        printf("Set channel: %d\r\n", i);
        //rda5981_set_channel((unsigned char)i);
        //rda5981_sniffer_set_channel((unsigned char)i);
        wland_sniffer_set_channel((unsigned char)i);
        //osDelay(200);
        ret = osSemaphoreWait(sw_mode.sema_id, 200);
        if(ret < 0) {
            printf("wait sema err\r\n");
            return -2;
        } else if(ret > 0) {
            printf("wait switch mode done, disable sniffer, use fixed channel 6\r\n");
            rda5981_disable_sniffer();
            rda5981_set_channel(6);
            //wland_sniffer_set_channel(6);
            break;
        }
        i++;
        if(i > 13) {
            i = 1;
        }
    } while(1);


    while (true) {
        rda_msg_get(main_msgQ, &msg, osWaitForever);
        switch(msg)
        {
            case MAIN_RECONNECT: {
                printf("wifi disconnect!\r\n");
                ret = wifi.disconnect();
                if(ret != 0){
                    printf("disconnect failed!\r\n");
                    break;
                }
                ret = wifi.reconnect();
                while(ret != 0){
                    if(++cnt>5)
                        break;
                    osDelay(cnt*2*1000);
                    ret = wifi.reconnect();
                };
                cnt = 1;
                break;
            }
            case MAIN_STOP_AP:
                wifi.stop_ap(1);
                break;
            default:
                printf("unknown msg\r\n");
                break;
        }
    }
}
