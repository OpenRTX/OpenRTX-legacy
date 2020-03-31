
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "usb_vcp.h"

int main (void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    GPIOE->MODER |= (1 << 2) | 1;

    if(RCC->CR & RCC_CR_HSERDY) GPIOE->ODR ^= (1 << 0);
    if((RCC->CFGR & RCC_CFGR_SWS ) == RCC_CFGR_SWS_PLL) GPIOE->ODR ^= (1 << 1);

    TM_USB_VCP_Init();
    uint8_t c;

    while (1)
    {
        if (TM_USB_VCP_GetStatus() == TM_USB_VCP_CONNECTED)
        {
            if (TM_USB_VCP_Getc(&c) == TM_USB_VCP_DATA_OK)
            {
                TM_USB_VCP_Putc(c);
            }
        }
        else
        {
        }

        vTaskDelay(50);
    }
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
