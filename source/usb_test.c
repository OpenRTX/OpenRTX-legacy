
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usb_vcp.h"
#include "gpio.h"

static void led(void*);
static void print(void*);

int main (void)
{
    gpio_setMode(GPIOE, 0, OUTPUT);
    gpio_setMode(GPIOE, 1, OUTPUT);

    TM_USB_VCP_Init();

    xTaskCreate(led, "grn", 256, NULL, 1, NULL);
    xTaskCreate(print, "red", 256, NULL, 0, NULL);
    vTaskStartScheduler();

    for(;;);
}

static void print(void* p)
{
    for(;;)
    {
        TM_USB_VCP_Puts("Test\r\n\0");
        vTaskDelay(1000);
    }
}

static void led(void* p)
{
    for(;;)
    {
        gpio_togglePin(GPIOE, 1);
        vTaskDelay(500);
    }
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
