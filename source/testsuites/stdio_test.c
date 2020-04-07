
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

    printf("Vcom ok\r\n");

    while(1)
    {
        char buf[50];
        scanf("%s", buf);
        printf("%s\r\n", buf);
    }
}
