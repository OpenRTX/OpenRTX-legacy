
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "usb_vcom.h"
#include "gpio.h"

void uDelay (const uint32_t usec)
{

    uint32_t count = 0;
    const uint32_t utime = (120 * usec / 7);

    do
    {
        if( ++count > utime )
        {
            return ;
        }
    } while (1);
}

void fillBuf(char *buf, size_t len)
{
    for(size_t i = 0; i < len; i++)
    {
        buf[i] = '!' + (i % 126);
    }
}

char buf[1030] = {0};

void sendTestVector(size_t n)
{
    int len = sprintf(buf, "\r\nTesting with %zd characters: ", n);
    vcom_writeBlock(buf, len);
    fillBuf(buf, n);
    vcom_writeBlock(buf, n);
}

int main (void)
{
    gpio_setMode(GPIOE, 0, OUTPUT);
    gpio_setMode(GPIOE, 1, OUTPUT);

    vcom_init();

    gpio_setPin(GPIOE, 0);

    size_t sz = 8;
    while(sz < 256)
    {
        sendTestVector(sz);
        sz *= 2;
        uDelay(500*1000);
    }

    gpio_clearPin(GPIOE, 0);

    for(;;) ;
}
