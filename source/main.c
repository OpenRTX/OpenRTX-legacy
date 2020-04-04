
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "stm32f4xx.h"
#include "keyboard.h"
#include "usb_vcp.h"

static void sleep(uint32_t);

static void green_main(void*);
static void fw_main_task(void*);

int main (void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    GPIOE->MODER |= (1 << 2) | 1;

    xTaskCreate(green_main, "grn", 256, NULL, 1, NULL);
    xTaskCreate(fw_main_task, "main", 256, NULL, 2, NULL);
    vTaskStartScheduler();
}

static void fw_main_task(void* data) {
	keyboardCode_t keys;
	int key_event;

    for(;;) {
        fw_check_key_event(&keys, &key_event); // Read keyboard state and event
        if (key_event != EVENT_KEY_NONE)
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

static void sleep(uint32_t ms)
{
    vTaskDelay(ms);
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
