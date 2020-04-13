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
#include "graphics.h"
#include "lcd.h"

void blink(void *arg)
{

    graphicsInit(COLOR_WHITE);
    clearRows(0, 8, COLOR_WHITE);
    lcd_setBacklightLevel(254);

    /* Horizontal red line */
    for(uint8_t x = 0; x < SCREEN_WIDTH; x++)
    {
        for(uint8_t y = 10; y < 30; y++)
        {
            setPixel(x, y, 0xF800);             /* RED */
        }
    }

    /* Vertical blue line */
    for(uint8_t x = 10; x < 30; x++)
    {
        for(uint8_t y = 0; y < SCREEN_HEIGHT; y++)
        {
            setPixel(x, y, 0x001F);             /* BLUE */
        }
    }

    /* Vertical green line */
    for(uint8_t x = 80; x < 100; x++)
    {
        for(uint8_t y = 0; y < SCREEN_HEIGHT; y++)
        {
            setPixel(x, y, 0x07e0);             /* GREEN */
        }
    }

    drawLine(0, 0, 100, 100, 0x1234);
    drawRect(100, 100, 20, 20, 0x0056);
    fillRect(30, 30, 60, 60, 0x0000);
    char *buffer = "KEK";
    //printCore(0, 3, buffer, FONT_SIZE_4, TEXT_ALIGN_RIGHT, 0x0000);
    //renderRows(4, 8);
    //renderRows(0, 4);
    render();

    while(1)
    {
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
