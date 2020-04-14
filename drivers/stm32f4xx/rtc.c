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

#include "rtc.h"
#include <stdio.h>

void rtc_init()
{
    /* Enable backup domain write protection */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RCC->BDCR = RCC_BDCR_RTCEN      /* Enable RTC              */
              | RCC_BDCR_RTCSEL_0   /* Set LSE as clock source */
              | RCC_BDCR_LSEON;     /* Enable LSE              */

    printf("Starting LSE... ");
    /* Wait until external 32kHz crystal stabilises */
    while((RCC->BDCR & RCC_BDCR_LSERDY) == 0) ;
    puts("OK\r");
}

void rtc_shutdown()
{
    RCC->BDCR &= ~ RCC_BDCR_RTCEN | RCC_BDCR_LSEON;
}

void rtc_setTime(curTime_t t)
{
    uint32_t date = ((t.year  / 10) << 20)
                  | ((t.year  % 10) << 16)
                  | ((t.month / 10) << 12)
                  | ((t.month % 10) << 8)
                  | ((t.date  / 10) << 4)
                  |  (t.date  % 10);
    date &= RTC_DR_YT | RTC_DR_YU | RTC_DR_MT | RTC_DR_MU | RTC_DR_DT | RTC_DR_DU;

    uint32_t time = ((t.hour   / 10) << 20)
                  | ((t.hour   % 10) << 16)
                  | ((t.minute / 10) << 12)
                  | ((t.minute % 10) << 8)
                  | ((t.second / 10) << 4)
                  |  (t.second % 10);
    time &= RTC_TR_HT | RTC_TR_HU | RTC_TR_MNT | RTC_TR_MNU | RTC_TR_ST | RTC_TR_SU;

    /* Enter initialisation mode and update registers */
    printf("Putting RTC in initialisation mode... ");
    RTC->ISR |= RTC_ISR_INIT;
    while((RTC->ISR & RTC_ISR_INITF) == 0) ;
    RTC->TR = time;
    RTC->DR = date;
    RTC->ISR &= ~RTC_ISR_INIT;
    puts("OK\r");
}

void rtc_setHour(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    curTime_t t = rtc_getTime();
    t.hour   = hours;
    t.minute = minutes;
    t.second = seconds;
    rtc_setTime(t);
}

void rtc_setDate(uint8_t date, uint8_t month, uint8_t year)
{
    curTime_t t = rtc_getTime();
    t.date  = date;
    t.month = month;
    t.year  = year;
    rtc_setTime(t);
}

curTime_t rtc_getTime()
{
    curTime_t t;

    uint32_t time = RTC->TR;
    t.hour   = ((time & RTC_TR_HT)  >> 20)*10 + ((time & RTC_TR_HU) >> 16);
    t.minute = ((time & RTC_TR_MNT) >> 12)*10 + ((time & RTC_TR_MNU) >> 8);
    t.second = ((time & RTC_TR_ST)  >> 4)*10  + (time & RTC_TR_SU);

    uint32_t date = RTC->DR;
    t.year  = ((date & RTC_DR_YT)  >> 20)*10 + ((date & RTC_DR_YU) >> 16);
    t.day   = ((date & RTC_DR_WDU) >> 13);
    t.month = ((date & RTC_DR_MT)  >> 12)*10 + ((date & RTC_DR_MU) >> 8);
    t.date  = ((date & RTC_DR_DT)  >> 4)*10  + (date & RTC_DR_DU);

    return t;
}
