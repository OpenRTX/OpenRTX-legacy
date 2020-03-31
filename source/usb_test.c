
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "usb_cdc.h"

int main (void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    GPIOE->MODER |= (1 << 2) | 1;
    GPIOE->ODR ^= (1 << 0);

    usb_cdc_init();

  while (1)
  {
      const char *str = "USB test\r\n\0";
      usb_cdc_write((str), sizeof(str));
      vTaskDelay(1000);
  }
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
