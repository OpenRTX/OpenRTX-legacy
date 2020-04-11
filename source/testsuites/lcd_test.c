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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "gpio.h"
#include "lcd.h"

void blink(void *arg)
{
    lcd_init();
    lcd_setBacklightLevel(254);
    uint16_t *buf = lcd_getFrameBuffer();

    /* All screen white */
    for(uint8_t x = 0; x < SCREEN_WIDTH; x++)
    {
        for(uint8_t y = 0; y < SCREEN_HEIGTH; y++)
        {
            buf[x+y*SCREEN_WIDTH] = 0xFFFF;     /* WHITE */
        }
    }

    /* Horizontal red line */
    for(uint8_t x = 0; x < SCREEN_WIDTH; x++)
    {
        for(uint8_t y = 10; y < 30; y++)
        {
            buf[x+y*SCREEN_WIDTH] = 0xF800;     /* RED */
        }
    }

    /* Vertical blue line */
    for(uint8_t x = 10; x < 30; x++)
    {
        for(uint8_t y = 0; y < SCREEN_HEIGTH; y++)
        {
            buf[x+y*SCREEN_WIDTH] = 0x001F;     /* BLUE */
        }
    }


    while(1)
    {
        lcd_render();
        gpio_togglePin(GPIOE, 0);
        vTaskDelay(500);
    }
}

int main (void)
{
    gpio_setMode(GPIOE, 0, OUTPUT);

    xTaskCreate(blink, "blink", 256, NULL, 0, NULL);
    vTaskStartScheduler();

    for(;;) ;
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
