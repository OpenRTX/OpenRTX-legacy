
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "gpio.h"

void testAllocation(void *arg)
{
    size_t chunk = 8;
    while(chunk <= 4096)
    {
        printf("Allocating a chunk of %zd bytes... ", chunk);
        uint8_t *p = ((uint8_t *) malloc(chunk));
        if(p != NULL)
        {
            puts("OK");
            free(p);
        }
        else
        {
            puts("FAIL");
        }
        chunk *= 2;
        vTaskDelay(250);
    }

    puts("End of test");
    for(;;) ;
}

void blink(void *arg)
{
    while(1)
    {
        gpio_togglePin(GPIOE, 0);
        vTaskDelay(500);
    }
}

int main (void)
{
    gpio_setMode(GPIOE, 0, OUTPUT);

    xTaskCreate(testAllocation, "testAllocation", 256, NULL, 1, NULL);
    xTaskCreate(blink, "blink", 256, NULL, 0, NULL);
    vTaskStartScheduler();

    for(;;) ;
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
