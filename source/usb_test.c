
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE usbDev __ALIGN_END;
USBD_Class_cb_TypeDef  USBD_CDC_cb;

void OTG_FS_IRQHandler(void)
{
    USBD_OTG_ISR_Handler(&usbDev);
}

int main (void)
{
    USBD_Init(&usbDev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);

  while (1)
  {
      const char *str = "USB test\r\n\0";
      VCP_ReceiveData(&usbDev, (uint8_t*)(str), sizeof(str));
      vTaskDelay(1000);
  }
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
