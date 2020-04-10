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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "gpio.h"
#include "lcd.h"
#include "delays.h"

/* Really ugly but useful defines */
#define D0  GPIOD,14
#define D1  GPIOD,15
#define D2  GPIOD,0
#define D3  GPIOD,1
#define D4  GPIOE,7
#define D5  GPIOE,8
#define D6  GPIOE,9
#define D7  GPIOE,10
#define WR  GPIOD,5
#define RD  GPIOD,4
#define CS  GPIOD,6
#define RS  GPIOD,12
#define RST GPIOD,13

/**
 * LCD command set, basic and extended
 */

#define CMD_NOP          0x00 // No Operation
#define CMD_SWRESET      0x01 // Software reset
#define CMD_RDDIDIF      0x04 // Read Display ID Info
#define CMD_RDDST        0x09 // Read Display Status
#define CMD_RDDPM        0x0a // Read Display Power
#define CMD_RDD_MADCTL   0x0b // Read Display
#define CMD_RDD_COLMOD   0x0c // Read Display Pixel
#define CMD_RDDDIM       0x0d // Read Display Image
#define CMD_RDDSM        0x0e // Read Display Signal
#define CMD_RDDSDR       0x0f // Read display self-diagnostic resut
#define CMD_SLPIN        0x10 // Sleep in & booster off
#define CMD_SLPOUT       0x11 // Sleep out & booster on
#define CMD_PTLON        0x12 // Partial mode on
#define CMD_NORON        0x13 // Partial off (Normal)
#define CMD_INVOFF       0x20 // Display inversion off
#define CMD_INVON        0x21 // Display inversion on
#define CMD_GAMSET       0x26 // Gamma curve select
#define CMD_DISPOFF      0x28 // Display off
#define CMD_DISPON       0x29 // Display on
#define CMD_CASET        0x2a // Column address set
#define CMD_RASET        0x2b // Row address set
#define CMD_RAMWR        0x2c // Memory write
#define CMD_RGBSET       0x2d // LUT parameter (16-to-18 color mapping)
#define CMD_RAMRD        0x2e // Memory read
#define CMD_PTLAR        0x30 // Partial start/end address set
#define CMD_VSCRDEF      0x31 // Vertical Scrolling Direction
#define CMD_TEOFF        0x34 // Tearing effect line off
#define CMD_TEON         0x35 // Tearing effect mode set & on
#define CMD_MADCTL       0x36 // Memory data access control
#define CMD_VSCRSADD     0x37 // Vertical scrolling start address
#define CMD_IDMOFF       0x38 // Idle mode off
#define CMD_IDMON        0x39 // Idle mode on
#define CMD_COLMOD       0x3a // Interface pixel format
#define CMD_RDID1        0xda // Read ID1
#define CMD_RDID2        0xdb // Read ID2
#define CMD_RDID3        0xdc // Read ID3

#define CMD_SETOSC       0xb0 // Set internal oscillator
#define CMD_SETPWCTR     0xb1 // Set power control
#define CMD_SETDISPLAY   0xb2 // Set display control
#define CMD_SETCYC       0xb4 // Set dispaly cycle
#define CMD_SETBGP       0xb5 // Set BGP voltage
#define CMD_SETVCOM      0xb6 // Set VCOM voltage
#define CMD_SETEXTC      0xb9 // Enter extension command
#define CMD_SETOTP       0xbb // Set OTP
#define CMD_SETSTBA      0xc0 // Set Source option
#define CMD_SETID        0xc3 // Set ID
#define CMD_SETPANEL     0xcc // Set Panel characteristics
#define CMD_GETHID       0xd0 // Read Himax internal ID
#define CMD_SETGAMMA     0xe0 // Set Gamma
#define CMD_SET_SPI_RDEN 0xfe // Set SPI Read address (and enable)
#define CMD_GET_SPI_RDEN 0xff // Get FE A[7:0] parameter

#define LCD_DELAY_US 30

// LCD framebuffer, allocated on the heap by lcd_init().
// Pixel format is RGB565, 16 bit per pixel
static uint16_t *frameBuffer;

static inline void writeCmd(uint8_t cmd)
{
    /*
     * HACK: to make things faster, we control GPIOs writing directly to the
     * control registers.
     */
    GPIOD->BSRRL = (1 << 4);              /* Set RD */
    GPIOD->BSRRH = 0xD023;                /* Clear D0, D1, D2, D3, WR, RS */
    GPIOE->BSRRH = 0x0780;                /* Clear D4, D5, D6, D7 */
    uint16_t x = cmd;
    GPIOD->BSRRL = ((x << 14) & 0xC000)   /* Set D0, D1 */
                 | ((x >> 2) & 0x0003);   /* D2, D3 */
    GPIOE->BSRRL = (x << 3) & 0x0780;     /* Set D4, D5, D6, D7 */
    delayUs(LCD_DELAY_US);
    GPIOD->BSRRL = (1 << 5);              /* Set WR line */
    delayUs(LCD_DELAY_US);
}

static inline void writeData(uint8_t val)
{
    /*
     * HACK: to make things faster, we control GPIOs writing directly to the
     * control registers.
     */
    GPIOD->BSRRL = (1 << 12) | (1 << 4);  /* Set RD and RS */
    GPIOD->BSRRH = 0xC023;                /* Clear D0, D1, D2, D3, WR */
    GPIOE->BSRRH = 0x0780;                /* Clear D4, D5, D6, D7 */
    uint16_t x = val;
    GPIOD->BSRRL = ((x << 14) & 0xC000)   /* Set D0, D1 */
                 | ((x >> 2) & 0x0003);   /* D2, D3 */
    GPIOE->BSRRL = (x << 3) & 0x0780;     /* Set D4, D5, D6, D7 */
    delayUs(LCD_DELAY_US);
    GPIOD->BSRRL = (1 << 5);              /* Set WR line */
    delayUs(LCD_DELAY_US);
}

void lcd_init()
{
    /* Allocate framebuffer, two bytes per pixel */
    frameBuffer = (uint16_t *) malloc(SCREEN_WIDTH * SCREEN_HEIGTH * 2);
    if(frameBuffer == NULL)
    {
        printf("*** LCD ERROR: cannot allocate framebuffer! ***");
        return;
    }

    /*
     * Configure TIM8 for backlight PWM: Fpwm = 100kHz, 8 bit of resolution
     * APB2 freq. is 84MHz, then: PSC = 327 to have Ftick = 256.097kHz
     * With ARR = 256, Fpwm is 100kHz;
     */
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    TIM8->ARR = 255;
    TIM8->PSC = 327;
    TIM8->CNT = 0;
    TIM8->CR1   |= TIM_CR1_ARPE;    /* LCD backlight is on PC6, TIM8-CH1 */
    TIM8->CCMR1 |= TIM_CCMR1_OC1M_2
                |  TIM_CCMR1_OC1M_1
                |  TIM_CCMR1_OC1PE;
    TIM8->CCER  |= TIM_CCER_CC1E;
    TIM8->BDTR  |= TIM_BDTR_MOE;
    TIM8->CCR1 = 0;
    TIM8->EGR  = TIM_EGR_UG;        /* Update registers */
    TIM8->CR1 |= TIM_CR1_CEN;       /* Start timer */

    /* Configure GPIO, TIM8 is on AF3 */
    gpio_setMode(GPIOC, 6, ALTERNATE);
    gpio_setAlternateFunction(GPIOC, 6, 3);

    gpio_setMode(D0, OUTPUT);
    gpio_setMode(D1, OUTPUT);
    gpio_setMode(D2, OUTPUT);
    gpio_setMode(D3, OUTPUT);
    gpio_setMode(D4, OUTPUT);
    gpio_setMode(D5, OUTPUT);
    gpio_setMode(D6, OUTPUT);
    gpio_setMode(D7, OUTPUT);

    gpio_clearPin(D0);
    gpio_clearPin(D1);
    gpio_clearPin(D2);
    gpio_clearPin(D3);
    gpio_clearPin(D4);
    gpio_clearPin(D5);
    gpio_clearPin(D6);
    gpio_clearPin(D7);

    gpio_setMode(WR,  OUTPUT);
    gpio_setMode(RD,  OUTPUT);
    gpio_setMode(CS,  OUTPUT);
    gpio_setMode(RS,  OUTPUT);
    gpio_setMode(RST, OUTPUT);

    gpio_setPin(WR);    /* Idle state is high level, for these */
    gpio_setPin(RD);
    gpio_setPin(CS);
    gpio_setPin(RS);

    gpio_clearPin(RST); /* Put LCD in reset mode */

    delayMs(20);
    gpio_setPin(RST);   /* Exit from reset */

    /* Configure LCD controller */
    gpio_clearPin(CS);
    writeCmd(CMD_SLPOUT);
    delayMs(120);
//     writeCmd(CMD_GAMSET);
//     writeData(0x04);
    writeCmd(CMD_MADCTL);
    writeData(0x22);
    writeCmd(CMD_CASET);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0xA0);    /* 128 coloumns */
    writeCmd(CMD_RASET);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x80);    /* 160 rows */
//     writeCmd(CMD_SETPWCTR);
//     writeData(0x0A);
//     writeData(0x14);
//     writeCmd(CMD_SETSTBA);
//     writeData(0x0A);
//     writeData(0x00);
    writeCmd(CMD_COLMOD);
    writeData(0x05);    /* 16 bit per pixel */
    delayMs(10);
//     writeCmd(CMD_RASET);
//     writeData(0x00);
//     writeData(0x00);
//     writeData(0x00);
//     writeData(0x79);
    writeCmd(CMD_NORON);
    delayMs(10);
    writeCmd(CMD_DISPON);
    delayMs(120);
    writeCmd(CMD_RAMWR);

    gpio_setPin(CS);
}

void lcd_terminate()
{
    /* Shut off backlight and deallocate framebuffer */
    gpio_setMode(GPIOC, 6, OUTPUT);
    gpio_clearPin(GPIOC, 6);
    RCC->APB2ENR &= ~RCC_APB2ENR_TIM8EN;
    if(frameBuffer != NULL) free(frameBuffer);
}

void lcd_setBacklightLevel(uint8_t level)
{
    TIM8->CCR1 = level;
}

void lcd_render()
{
    gpio_clearPin(CS);
    gpio_setMode(D0, OUTPUT);
    gpio_setMode(D1, OUTPUT);
    gpio_setMode(D2, OUTPUT);
    gpio_setMode(D3, OUTPUT);
    gpio_setMode(D4, OUTPUT);
    gpio_setMode(D5, OUTPUT);
    gpio_setMode(D6, OUTPUT);
    gpio_setMode(D7, OUTPUT);
    gpio_setMode(WR, OUTPUT);
    gpio_setMode(RD, OUTPUT);

    writeCmd(CMD_RAMWR);

    for(uint8_t r = 0; r < 128; r++)
    {
        for(uint8_t c = 0; c < 160; c++)
        {
            frameBuffer[r + c*128] = (c % 2) ? 0xF800 : 0x001F;
        }
    }

    for(size_t p = 0; p < 160*128; p++)
    {
            uint16_t pix = frameBuffer[p];
            writeData(pix & 0xFF);
            writeData(pix >> 8);
    }
    gpio_setPin(CS);
}
