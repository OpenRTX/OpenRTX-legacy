
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

int main (void)
{
    gpio_setMode(GPIOE, 0, OUTPUT);
    gpio_setMode(GPIOE, 1, OUTPUT);

    vcom_init();

    char *buf[50] = {0};

    for(;;)
    {
        ssize_t len = vcom_readBlock(buf, 50);
        if(len > 0)
        {
            buf[len] = '\0';

            char str[80];
            int n = snprintf(str, 80, "Got %zd characters: %s\r\n", len, buf);
            vcom_writeBlock(str, n);
        }
        gpio_togglePin(GPIOE, 1);
        uDelay(500*1000);
    }

}
