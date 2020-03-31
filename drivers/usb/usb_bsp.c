/**
  ******************************************************************************
  * @file    usb_bsp.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    17-March-2018
  * @brief   This file is responsible to offer board support package and is 
  *          configurable by user.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      <http://www.st.com/SLA0044>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------ */
#include "usb_bsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
* @{
*/

/** @defgroup USB_BSP
* @brief This file is responsible to offer board support package
* @{
*/

/** @defgroup USB_BSP_Private_Defines
* @{
*/
/**
* @}
*/


/** @defgroup USB_BSP_Private_TypesDefinitions
* @{
*/
/**
* @}
*/





/** @defgroup USB_BSP_Private_Macros
* @{
*/
/**
* @}
*/

/** @defgroup USBH_BSP_Private_Variables
* @{
*/

/**
* @}
*/

/** @defgroup USBH_BSP_Private_FunctionPrototypes
* @{
*/
/**
* @}
*/

/** @defgroup USB_BSP_Private_Functions
* @{
*/


/**
* @brief  USB_OTG_BSP_Init
*         Initializes BSP configurations
* @param  None
* @retval None
*/

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE * pdev)
{
    // PA9 USB_VBUS
    // PA11 USB_DM
    // PA12 USB_DP

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER   |= (0x02 << 2*11)   | (0x02 << 2*12);  // Alternate mode
    GPIOA->AFR[1]  |= (0x0E << 4*3)    | (0x0E << 4*4);   // AF14
    GPIOA->PUPDR   &= ~((0x03 << 2*11) | (0x03 << 2*12)); // No pull-up/pull-down
    GPIOA->OSPEEDR |= (0x03 << 2*11)   | (0x03 << 2*12);  // High speed

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;

    //TODO: fix priority definition
    NVIC_SetPriority(OTG_FS_IRQn, ((configMAX_SYSCALL_INTERRUPT_PRIORITY) >> 4));
}

/**
* @brief  USB_OTG_BSP_EnableInterrupt
*         Enable USB Global interrupt
* @param  None
* @retval None
*/
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE * pdev)
{
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

/**
* @brief  USB_OTG_BSP_uDelay
*         This function provides delay time in micro sec
* @param  usec : Value of delay required in micro sec
* @retval None
*/

void USB_OTG_BSP_uDelay(const uint32_t usec)
{
    uint32_t t = usec;
    while (t > 1000)
    {
        USB_OTG_BSP_mDelay(1);
        t -= 1000;
    }

    const uint32_t bound = t * 168;
    for (uint32_t j = 0; j < bound; j+= 4)
    {
        asm volatile ("mov r0,r0");
    }
}


/**
* @brief  USB_OTG_BSP_mDelay
*          This function provides delay time in milli sec
* @param  msec : Value of delay required in milli sec
* @retval None
*/
void USB_OTG_BSP_mDelay(const uint32_t msec)
{
    vTaskDelay(msec);
}

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
