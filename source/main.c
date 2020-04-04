
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "stm32f4xx.h"

static void sleep(uint32_t);

static void green_main(void*);
static void red_main(void*);

int main (void)
{
	keyboardCode_t keys;
	int key_event;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    GPIOE->MODER |= (1 << 2) | 1;

    xTaskCreate(green_main, "grn", 256, NULL, 1, NULL);
    xTaskCreate(red_main, "red", 256, NULL, 0, NULL);
    xTaskCreate(fw_main_task, "main", 256, NULL, 2, NULL);
    vTaskStartScheduler();

}

static void fw_main_task(void* data) {
    for(;;) {
        fw_check_key_event(&keys, &key_event); // Read keyboard state and event
        if (key_event != NO_EVENT)
            TM_USB_VCP_Puts("A key was pressed!\r\n\0");
    }
}

static void green_main(void* machtnichts) {

    for(;;)
    {
        GPIOE->ODR ^= (1 << 0); // PE0
        sleep(1000);
    }
}

static void red_main(void* machtnichts) {

    for(;;)
    {
        GPIOE->ODR ^= (1 << 1); // PE1
        sleep(500);
    }
}

static void sleep(uint32_t ms)
{
    vTaskDelay(ms);
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
