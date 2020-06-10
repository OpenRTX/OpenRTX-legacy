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

#include "pll.h"
#include <math.h>
#include "gpio.h"
#include "delays.h"

#define REF_CLK 16800000.0F  /* Reference clock: 16.8MHz                 */
#define PHD_GAIN 0x1F        /* Phase detector gain: hex value, max 0x1F */

void spiSend(uint16_t value)
{
    uint16_t temp = value;

    /*
     * PD11 -> PLL chip select (active low)
     * PE4 -> PLL clock
     * PE5 -> PLL data
     */

    gpio_clearPin(GPIOD, 11);
    delayUs(10);

    for(uint8_t i = 0; i < 16; i++)
    {
        gpio_setPin(GPIOE, 4);
        delayUs(1);
        if(temp & 0x8000)
            gpio_setPin(GPIOE, 5);
        else
            gpio_clearPin(GPIOE, 5);
        temp <<= 1;
        delayUs(1);
        gpio_clearPin(GPIOE, 4);
        delayUs(1);
    }

    gpio_setPin(GPIOE, 4);

    delayUs(10);
    gpio_setPin(GPIOD, 11);
}

void pll_init()
{
    gpio_setMode(GPIOE, 4, OUTPUT);     /* PLL clock */
    gpio_setMode(GPIOE, 5, OUTPUT);     /* PLL data  */
    gpio_setMode(GPIOD, 11, OUTPUT);    /* PLL cs    */
    gpio_setPin(GPIOD, 11);
    gpio_setMode(GPIOD, 10, INPUT);     /* PLL lock detect */

    spiSend(0x6000 | ((uint16_t) PHD_GAIN)); /* Phase detector gain                     */
    spiSend(0x73D0);                         /* Power down/multiplexer control register */
    spiSend(0x8000);                         /* Modulation control register             */
    spiSend(0x9000);                         /* Modulation data register                */
    pll_setFrequency(430000000, 4);
}

void pll_setFrequency(float freq, uint8_t clkDiv)
{
    /* Maximum allowable value for reference clock divider is 32 */
    if (clkDiv > 32) clkDiv = 32;

    float K = freq/(REF_CLK/((float) clkDiv));
    float Ndiv = floor(K) - 32.0;
    float Ndnd = round(262144*(K - Ndiv - 32.0));

    uint32_t dnd = ((uint32_t) Ndnd);
    uint16_t dndMsb = dnd >> 8;
    uint16_t dndLsb = dnd & 0x00FF;

    spiSend((uint16_t) Ndiv);                   /* Divider register      */
    spiSend(0x2000 | dndLsb);                   /* Dividend LSB register */
    spiSend(0x1000 | dndMsb);                   /* Dividend MSB register */
    spiSend(0x5000 | ((uint16_t)clkDiv - 1));   /* Reference clock divider */
}

bool pll_locked()
{
    return (gpio_readPin(GPIOD, 10) == 1) ? true : false;
}

bool pll_spiInUse()
{
    /*
     * If PLL chip select is low, SPI is being used by this driver.
     * We can check it by reading the GPIOD ODR.
     */
    return (GPIOD->ODR & (1 << 11)) ? false : true;
}

