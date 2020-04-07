/***************************************************************************
 *   Copyright (C) 2020 by Silvano Seva IU2KWO                             *
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

/**
 * Memory allocation test: this program allocates on the heap chunks of
 * increasing size up to 2kB and writes random values to them.
 * Test is successful if system does not hang (green led keeps blinking) and no
 * errors arise both in memory allocation and writing.
 */

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
            printf("OK. Writing to chunk... ");

            uint32_t a = 16807;
            uint32_t m = 2147483647;

            for(size_t i = 0; i < chunk; i++)
            {
                uint32_t seed = (a * chunk) % m;
                p[i] = ((uint8_t) (seed / m) % 0xFF);
            }

            puts("OK\r");
            free(p);
        }
        else
        {
            puts("FAIL\r");
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
