
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "usb_vcp.h"
#include "usb_bsp.h"

int main (void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    GPIOE->MODER |= (1 << 2) | 1;

    TM_USB_VCP_Init();

    while (1)
    {
        TM_USB_VCP_Puts("Test\r\n\0");
        USB_OTG_BSP_mDelay(500);
    }
}
