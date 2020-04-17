/***************************************************************************
 *   Copyright (C) 2020 by Silvano Seva IU2KWO and Niccolò Izzo IU2KIN     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "rtc.h"
#include "gpio.h"
#include "adc1.h"

void task(void *arg)
{
    gpio_setMode(GPIOA, 9, OUTPUT);     // VCOVCC should be high when receiving
    gpio_setPin(GPIOA, 9);              // (see page 6 of schematic)

    gpio_setMode(GPIOA, 8, OUTPUT);     // Turn on VCO power supply
    gpio_setPin(GPIOA, 8);

    gpio_setMode(GPIOC, 9, OUTPUT);     // Turn on RX front-end power supply
    gpio_setPin(GPIOC, 9);

    gpio_setMode(GPIOB, 2, OUTPUT);     // Turn on AF amplifier
    gpio_setPin(GPIOB, 2);

    gpio_setMode(GPIOE, 13, OUTPUT);    // Unmute path from AF_out to speaker
    gpio_setPin(GPIOE, 13);

    gpio_setMode(GPIOB, 8, OUTPUT);     // Turn on speaker
    gpio_clearPin(GPIOB, 8);

    gpio_setMode(GPIOB, 9, OUTPUT);     // Turn on audio amplifier
    gpio_setPin(GPIOB, 9);


    adc1_init();

    while(1)
    {
        printf("RSSI value: %f\r\n", adc1_getMeasurement(1));
        vTaskDelay(500);
    }
}

int main (void)
{
    printf("** RX test** \r\n");

    xTaskCreate(task, "task", 256, NULL, 0, NULL);
    vTaskStartScheduler();

    for(;;) ;
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
