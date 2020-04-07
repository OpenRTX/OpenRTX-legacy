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
 * Validation test to verify that virtual COM port driver can autonomously
 * transmit data chunks whose length is more than a USB transfer size,
 * consisting of 48 bytes.
 */

#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "usb_vcom.h"
#include "gpio.h"

void uDelay (const uint32_t usec)
{

    uint32_t count = 0;
    const uint32_t utime = (120 * usec / 7);

    do
    {
        if( ++count > utime )
        {
            return ;
        }
    } while (1);
}

void fillBuf(char *buf, size_t len)
{
    for(size_t i = 0; i < len; i++)
    {
        buf[i] = '!' + (i % 126);
    }
}

char buf[1030] = {0};

void sendTestVector(size_t n)
{
    int len = sprintf(buf, "\r\nTesting with %zd characters: ", n);
    vcom_writeBlock(buf, len);
    fillBuf(buf, n);
    vcom_writeBlock(buf, n);
}

int main (void)
{
    gpio_setMode(GPIOE, 0, OUTPUT);
    gpio_setMode(GPIOE, 1, OUTPUT);

    vcom_init();

    gpio_setPin(GPIOE, 0);

    size_t sz = 8;
    while(sz < 256)
    {
        sendTestVector(sz);
        sz *= 2;
        uDelay(500*1000);
    }

    gpio_clearPin(GPIOE, 0);

    for(;;) ;
}
