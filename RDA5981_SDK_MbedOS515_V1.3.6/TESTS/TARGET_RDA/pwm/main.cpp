#include "mbed.h"
#include "rtos.h"

PwmOut pwm0(PB_8);
PwmOut pwm1(PC_1);
PwmOut pwm2(PB_0);
PwmOut pwm3(PD_3);
PwmOut pwm4(PB_3);
PwmOut pwm7(PB_1);

int main() {
    printf("\r\nStart PWM test...\r\n");

    /* Set clock source & divider, PWM0~4 share the same source */
    //pwm0.clock_set(1, 0); // 10MHz, period: PWM0~3(10us~13.1ms)/PWM4(80us~818.4us)
    pwm0.clock_set(1, 4); // 2.5MHz, period: PWM0~3(40us~52.4ms)/PWM4(320us~3.27ms)

    /* Set period & duty */
    pwm0.period_ms(12);
    pwm0.write(0.125f);
    pwm1.period_ms(1);
    pwm1.write(0.25f);
    pwm2.period_us(400);
    pwm2.write(0.375f);
    pwm3.period_us(16);
    pwm3.write(0.5f);
    pwm4.period_us(200);
    pwm4.write(0.75f);
    pwm7.write(0.60f);

    while(true) {
        printf("duty=%f\r\n", pwm0.read());
        Thread::wait(1000);
    }
}

