#include "mbed.h"
#include "rtos.h"
#include "entropy_poll.h"

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

#define DATA_LEN    12

unsigned char dataout[DATA_LEN] = {0U};
unsigned int  datalen = 0U;

int main()
{
    int ret;

    printf("\r\nStart TRNG test...\r\n");

    ret = mbedtls_hardware_poll(NULL, dataout, DATA_LEN, &datalen);

    printf("ret=%d, datalen=%d\r\n", ret, datalen);
    for(ret = 0; ret < DATA_LEN; ret++){
        printf("%02X,", dataout[ret]);
    }
    printf("\r\n");

    osDelay(osWaitForever);
}

#else
#error "Hardware entropy collector disabled!"
#endif

