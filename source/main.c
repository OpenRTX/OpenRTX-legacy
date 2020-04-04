
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "stm32f4xx.h"
#include "keyboard.h"
#include "gpio.h"
#include "usb_vcp.h"

static void sleep(uint32_t);

static void green_main(void*);
static void fw_main_task(void*);

int main (void)
{
    gpio_setMode(GPIOE, 0, OUTPUT);
    gpio_setMode(GPIOE, 1, OUTPUT);

    TM_USB_VCP_Init();

    xTaskCreate(green_main, "grn", 256, NULL, 1, NULL);
    xTaskCreate(fw_main_task, "main", 256, NULL, 2, NULL);
    vTaskStartScheduler();
}

static void fw_main_task(void* data) {
	keyboardCode_t keys;
	int key_event;

    TM_USB_VCP_Puts("Keyboard test initialized!\r\n\0");
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
