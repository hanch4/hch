#include "mbed.h"
#include "rtos.h"
#include "WiFiStackInterface.h"
#include "rda_sys_wrapper.h"

int main() {
    int ret, cnt = 1;
    WiFiStackInterface wifi;
    const char *ip_addr;
    unsigned int msg;
    void *main_msgQ = rda_msgQ_create(5);

    printf("Start wifi_connect test...\r\n");

    //scan example
    rda5981_scan_result *bss_list = NULL;
    int scan_res;
    scan_res = wifi.scan(NULL, 0);
    bss_list = (rda5981_scan_result *)malloc(scan_res * sizeof(rda5981_scan_result));
    memset(bss_list, 0, scan_res * sizeof(rda5981_scan_result));
    if (bss_list == NULL) {
        printf("malloc buf fail\r\n");
        return -1;
    }
    ret = wifi.scan_result(bss_list, scan_res);
    printf("##########scan return :%d\n", ret);
    free(bss_list);

    //connect example
    wifi.set_msg_queue(main_msgQ);
    wifi.connect("SSID", "password", NULL, NSAPI_SECURITY_NONE);
    //wifi.start_ap("a", NULL, 6);

    ip_addr = wifi.get_ip_address();
    if (ip_addr) {
        printf("Client IP Address is %s\r\n", ip_addr);
    } else {
        printf("No Client IP Address\r\n");
    }

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
