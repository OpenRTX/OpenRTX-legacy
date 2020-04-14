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

void blink(void *arg)
{
    rtc_init();
//     rtc_setDate(14,4,2020);
//     rtc_setHour(17,40,0);

    curTime_t time;
    time.date = 14;
    time.month = 4;
    time.year = 20;
    time.hour = 18;
    time.minute = 20;
    time.second = 0;
    rtc_setTime(time);

    while(1)
    {
        curTime_t t = rtc_getTime();
        printf("%d-%d-%d %d:%d:%d\r\n",t.date, t.month, t.year+2000,
                                       t.hour, t.minute, t.second);
        gpio_togglePin(GPIOE, 0);
        vTaskDelay(500);
    }
}

int main (void)
{
    printf("** RTC testsuite** \r\n");
    gpio_setMode(GPIOE, 0, OUTPUT);

    xTaskCreate(blink, "blink", 256, NULL, 0, NULL);
    vTaskStartScheduler();

    for(;;) ;
}

void vApplicationTickHook() { }
void vApplicationStackOverflowHook() { }
void vApplicationIdleHook() { }
void vApplicationMallocFailedHook() { }
