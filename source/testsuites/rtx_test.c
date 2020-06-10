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
#include "gpio.h"
#include "adc1.h"
#include "delays.h"
#include "pll.h"

#define IF_FREQ 49950000.0F  // Intermediate frequency: 49.95MHz

void task(void *arg)
{
    delayMs(4000);

    gpio_setMode(GPIOA, 8, OUTPUT);     // Turn on VCO power supply
    gpio_setPin(GPIOA, 8);

    gpio_setMode(GPIOA, 9, OUTPUT);     // VCOVCC should be high when receiving
    gpio_setPin(GPIOA, 9);              // (see page 6 of schematic)

    gpio_setMode(GPIOA, 10, OUTPUT);    // Clear DMR_SW
    gpio_clearPin(GPIOA, 10);

    gpio_setMode(GPIOA, 13, OUTPUT);    // Clear W/N switch
    gpio_clearPin(GPIOA, 13);

    gpio_setMode(GPIOB, 2, OUTPUT);     // Turn on AF amplifier (FM_SW)
    gpio_setPin(GPIOB, 2);

    gpio_setMode(GPIOB, 8, OUTPUT);     // Turn on speaker
    gpio_clearPin(GPIOB, 8);

    gpio_setMode(GPIOB, 9, OUTPUT);     // Turn on audio amplifier
    gpio_setPin(GPIOB, 9);

    gpio_setMode(GPIOB, 12, OUTPUT);    // V_CS high
    gpio_setPin(GPIOB, 12);

    gpio_setMode(GPIOC, 4, OUTPUT);     // RF_APC_SW low
    gpio_clearPin(GPIOC, 4);

    gpio_setMode(GPIOC, 5, OUTPUT);     // Turn off TX front-end power supply
    gpio_clearPin(GPIOC, 5);

    gpio_setMode(GPIOC, 7, OUTPUT);     // Set CTC/DCS_OUT to 0V
    gpio_clearPin(GPIOC, 7);

    gpio_setMode(GPIOC, 9, OUTPUT);
    gpio_clearPin(GPIOC, 9);

    gpio_setMode(GPIOE, 2, OUTPUT);     // DMR_CS high
    gpio_setPin(GPIOE, 2);

    gpio_setMode(GPIOE, 6, OUTPUT);     // Put DMR baseband in sleep
    gpio_setPin(GPIOE, 6);

    gpio_setMode(GPIOE, 13, OUTPUT);    // Unmute path from AF_out to speaker
    gpio_setPin(GPIOE, 13);

    gpio_setMode(GPIOE, 0, OUTPUT);     // LED

    adc1_init();
    pll_init();

    gpio_setMode(GPIOA, 4, INPUT_ANALOG);   // DAC requires analog connection
    gpio_setMode(GPIOA, 5, INPUT_ANALOG);

    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR = DAC_CR_EN2 | DAC_CR_EN1;
    DAC->DHR12R2 = 0x3e8;               // 0V to MOD2_BIAS
    DAC->DHR12R1 = 0x956;               // APC/TV voltage

    pll_setFrequency(430100000 - IF_FREQ, 5);     // 430.100MHz
    gpio_setPin(GPIOC, 9);              // Turn on RX front-end power supply

    while(1)
    {
        gpio_togglePin(GPIOE, 0);
        if(pll_locked()) puts("PLL LOCK!\r");
        printf("RSSI %.3f\r\n", adc1_getMeasurement(1));
        delayMs(5000);
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
