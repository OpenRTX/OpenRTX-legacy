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
#define CMD_SETCYC       0xb4 // Set display cycle
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

#define RENDER_WR_SETUP_DELAY 30
#define RENDER_NEXT_BYTE_DELAY 30

#define LCD_FSMC_ADDR_COMMAND 0x60000000
#define LCD_FSMC_ADDR_DATA    0x60040000 

/*
 * LCD framebuffer, allocated on the heap by lcd_init().
 * Pixel format is RGB565, 16 bit per pixel
 */
static uint16_t *frameBuffer;

static inline void writeCmd(uint8_t cmd)
{
    *(volatile uint8_t*)LCD_FSMC_ADDR_COMMAND = cmd;
//     /*
//      * HACK: to make things faster, we control GPIOs writing directly to the
//      * control registers.
//      */
//     GPIOD->BSRRL = (1 << 4);              /* Set RD */
//     GPIOD->BSRRH = 0xD023;                /* Clear D0, D1, D2, D3, WR, RS */
//     GPIOE->BSRRH = 0x0780;                /* Clear D4, D5, D6, D7 */
//     uint16_t x = cmd;
//     GPIOD->BSRRL = ((x << 14) & 0xC000)   /* Set D0, D1 */
//                  | ((x >> 2) & 0x0003);   /* D2, D3 */
//     GPIOE->BSRRL = (x << 3) & 0x0780;     /* Set D4, D5, D6, D7 */
//     delayUs(LCD_DELAY_US);
//     GPIOD->BSRRL = (1 << 5);              /* Set WR line */
//     delayUs(LCD_DELAY_US);
}

static inline void writeData(uint8_t val)
{
    *(volatile uint8_t*)LCD_FSMC_ADDR_DATA = val;
//     /*
//      * HACK: to make things faster, we control GPIOs writing directly to the
//      * control registers.
//      */
//     GPIOD->BSRRL = (1 << 12) | (1 << 4);  /* Set RD and RS */
//     GPIOD->BSRRH = 0xC023;                /* Clear D0, D1, D2, D3, WR */
//     GPIOE->BSRRH = 0x0780;                /* Clear D4, D5, D6, D7 */
//     uint16_t x = val;
//     GPIOD->BSRRL = ((x << 14) & 0xC000)   /* Set D0, D1 */
//                  | ((x >> 2) & 0x0003);   /* D2, D3 */
//     GPIOE->BSRRL = (x << 3) & 0x0780;     /* Set D4, D5, D6, D7 */
//     delayUs(LCD_DELAY_US);
//     GPIOD->BSRRL = (1 << 5);              /* Set WR line */
//     delayUs(LCD_DELAY_US);
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

    /* Configure backlight GPIO, TIM8 is on AF3 */
    gpio_setMode(GPIOC, 6, ALTERNATE);
    gpio_setAlternateFunction(GPIOC, 6, 3);

    /* Configure display GPIOs */
//     gpio_setMode(D0, OUTPUT);
//     gpio_setMode(D1, OUTPUT);
//     gpio_setMode(D2, OUTPUT);
//     gpio_setMode(D3, OUTPUT);
//     gpio_setMode(D4, OUTPUT);
//     gpio_setMode(D5, OUTPUT);
//     gpio_setMode(D6, OUTPUT);
//     gpio_setMode(D7, OUTPUT);
// 
//     gpio_clearPin(D0);
//     gpio_clearPin(D1);
//     gpio_clearPin(D2);
//     gpio_clearPin(D3);
//     gpio_clearPin(D4);
//     gpio_clearPin(D5);
//     gpio_clearPin(D6);
//     gpio_clearPin(D7);

    /* Configure FSMC as LCD driver.
     * BCR1 config:
     * - CBURSTRW  = 0: asynchronous write operation
     * - ASYNCWAIT = 0: NWAIT not taken into account when running asynchronous protocol
     * - EXTMOD    = 0: do not take into account values of BWTR register
     * - WAITEN    = 0: nwait signal disabled
     * - WREN      = 1: write operations enabled
     * - WAITCFG   = 0: nwait active one data cycle before wait state
     * - WRAPMOD   = 0: direct wrapped burst disabled
     * - WAITPOL   = 0: nwait active low
     * - BURSTEN   = 0: burst mode disabled
     * - FACCEN    = 1: NOR flash memory disabled
     * - MWID      = 1: 16 bit external memory device
     * - MTYP      = 2: NOR
     * - MUXEN     = 0: addr/data not multiplexed
     * - MBNEN     = 1: enable bank
     */
    RCC->AHB3ENR |= RCC_AHB3ENR_FSMCEN;
    FSMC_Bank1->BTCR[0] = 0x10D9;
                        //= FSMC_BCR1_EXTMOD
                        //| FSMC_BCR1_WREN
                        //| FSMC_BCR1_MBKEN;

    /* BTR1 config:
     * - ACCMOD  = 0: access mode A
     * - DATLAT  = 0: don't care in asynchronous mode
     * - CLKDIV  = 1: don't care in asynchronous mode, 0000 is reserved
     * - BUSTURN = 0: time between two consecutive write accesses: (1 + 2 + BUSTURN)*HCLK_period = 71.4ns > twc (66ns)
     * - DATAST  = 3: we must have LCD twrl < DATAST*HCLK_period: 15ns < 3*5.95 = 17.85ns
     * - ADDHLD  = 1: used only in mode D, 0000 is reserved
     * - ADDSET  = 1: address setup time 3*HCLK_period = 17.85ns
     */
    FSMC_Bank1->BTCR[1] = (0 << 28) /* ACCMOD */
                        | (0 << 24) /* DATLAT */
                        | (1 << 20) /* CLKDIV */
                        | (0 << 16) /* BUSTURN */
                        | (5 << 8)  /* DATAST */
                        | (1 << 4)  /* ADDHLD */
                        | 7;        /* ADDSET */

    gpio_setMode(D0, ALTERNATE);
    gpio_setMode(D1, ALTERNATE);
    gpio_setMode(D2, ALTERNATE);
    gpio_setMode(D3, ALTERNATE);
    gpio_setMode(D4, ALTERNATE);
    gpio_setMode(D5, ALTERNATE);
    gpio_setMode(D6, ALTERNATE);
    gpio_setMode(D7, ALTERNATE);
    gpio_setMode(RS, ALTERNATE);
    gpio_setMode(WR, ALTERNATE);
    gpio_setMode(RD, ALTERNATE);

    gpio_setAlternateFunction(D0, 12);
    gpio_setAlternateFunction(D1, 12);
    gpio_setAlternateFunction(D2, 12);
    gpio_setAlternateFunction(D3, 12);
    gpio_setAlternateFunction(D4, 12);
    gpio_setAlternateFunction(D5, 12);
    gpio_setAlternateFunction(D6, 12);
    gpio_setAlternateFunction(D7, 12);
    gpio_setAlternateFunction(RS, 12);
    gpio_setAlternateFunction(WR, 12);
    gpio_setAlternateFunction(RD, 12);

//     gpio_setMode(WR,  OUTPUT);
//     gpio_setMode(RD,  OUTPUT);
    gpio_setMode(CS,  OUTPUT);
//     gpio_setMode(RS,  OUTPUT);
    gpio_setMode(RST, OUTPUT);

//     gpio_setPin(WR);    /* Idle state is high level, for these */
//     gpio_setPin(RD);
    gpio_setPin(CS);
//     gpio_setPin(RS);

    gpio_clearPin(RST); /* Put LCD in reset mode */

    delayMs(20);
    gpio_setPin(RST);   /* Exit from reset */

    gpio_clearPin(CS);
//     writeCmd(CMD_COLMOD);
//     writeData(5);

//     writeCmd(CMD_MADCTL);
    //  writeData(8);
//     writeData(0x48);

    writeCmd(CMD_SET_SPI_RDEN);
    writeCmd(0xef);
    writeCmd(CMD_SETCYC);
    writeData(0);
    writeCmd(CMD_GET_SPI_RDEN);
    writeData(0x16);

    writeCmd(0xfd);
    // writeData(0x40);
    writeData(0x4f);

    writeCmd(0xa4);
    writeData(0x70);
    writeCmd(0xe7);
    writeData(0x94);
    writeData(0x88);
    writeCmd(0xea);
    writeData(0x3a);
    writeCmd(0xed);
    writeData(0x11);
    writeCmd(0xe4);
    writeData(0xc5);
    writeCmd(0xe2);
    writeData(0x80);
    writeCmd(0xa3);
    writeData(0x12);
    writeCmd(0xe3);
    writeData(7);
    writeCmd(0xe5);
    writeData(0x10);
    writeCmd(0xf0);
    writeData(0);
    writeCmd(0xf1);
    writeData(0x55);
    writeCmd(0xf2);
    writeData(5);
    writeCmd(0xf3);
    writeData(0x53);
    writeCmd(0xf4);
    writeData(0);
    writeCmd(0xf5);
    writeData(0);
    writeCmd(0xf7);
    writeData(0x27);
    writeCmd(0xf8);
    writeData(0x22);
    writeCmd(0xf9);
    writeData(0x77);
    writeCmd(0xfa);
    writeData(0x35);
    writeCmd(0xfb);
    writeData(0);
    writeCmd(0xfc);
    writeData(0);
    writeCmd(CMD_SET_SPI_RDEN);
    writeCmd(0xef);
    writeCmd(0xe9);
    writeData(0);
    delayMs(20);

    /* Configure LCD controller */
//     gpio_clearPin(CS);
//     writeCmd(CMD_SLPOUT);
//     delayMs(120);
//     writeCmd(CMD_NORON);
//     delayMs(10);
//     writeCmd(CMD_SETEXTC);
//     writeCmd(CMD_SETOSC);
//     writeData(0x34);      /* 50Hz in idle mode and 60Hz normal mode */
//     writeData(0x01);      /* Enable oscillator */

    /*
     * Configuring screen's memory access control: TYT MD380 has the screen
     * rotated by 90° degrees, so we have to exgange row and coloumn indexing.
     * Moreover, we need to invert the vertical updating order to avoid painting
     * an image from bottom to top (that is, horizontally mirrored).
     * For reference see, in HX8353-E datasheet, MADCTL description at page 149
     * and paragraph 6.2.1, starting at page 48.
     *
     * Current confguration:
     * - MY  (bit 7): 0 -> do not invert y direction
     * - MX  (bit 6): 1 -> invert x direction
     * - MV  (bit 5): 1 -> exchange x and y
     * - ML  (bit 4): 0 -> refresh screen top-to-bottom
     * - BGR (bit 3): 0 -> RGB pixel format
     * - SS  (bit 2): 0 -> refresh screen left-to-right
     * - bit 1 and 0: don't care
     */

    writeCmd(CMD_MADCTL);
    writeData(0x60);

    writeCmd(CMD_CASET);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0xA0);      /* 160 coloumns */
    writeCmd(CMD_RASET);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x80);      /* 128 rows */
    writeCmd(CMD_COLMOD);
    writeData(0x05);      /* 16 bit per pixel */
    delayMs(10);

//     writeCmd(CMD_DISPON); /* Finally, turn on display */
//     delayMs(120);
//     writeCmd(CMD_RAMWR);

    writeCmd(CMD_SLPOUT);
    delayMs(120);
    writeCmd(CMD_DISPON);
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

    /*
     * Put screen data lines back to output mode, since they are in common with
     * keyboard buttons and the keyboard driver sets them as inputs.
     */
//     gpio_setMode(D0, OUTPUT);
//     gpio_setMode(D1, OUTPUT);
//     gpio_setMode(D2, OUTPUT);
//     gpio_setMode(D3, OUTPUT);
//     gpio_setMode(D4, OUTPUT);
//     gpio_setMode(D5, OUTPUT);
//     gpio_setMode(D6, OUTPUT);
//     gpio_setMode(D7, OUTPUT);

    gpio_clearPin(CS);
    writeCmd(CMD_RAMWR);
//     *(volatile uint8_t*)LCD_FSMC_ADDR_COMMAND = CMD_RAMWR;

    /*
     * Copying data from framebuffer to screen buffer. When using the 8-bit bus
     * interface, display expects values in order R-G-B, while in framebuffer
     * data is stored with blue component in the low 5 bits. Thus, we have to
     * send the high byte followed by the low one.
     * See also HX8353-E datasheed, at page 27.
     *
     * Also, to make things faster, we bypass the GPIO driver and write directly
     * into the GPIO control registers
     */

    GPIOD->BSRRL = (1 << 12) | (1 << 4);  /* Set RD and RS */

    for(size_t p = 0; p < 160*128; p++)
    {
        uint16_t rg = (frameBuffer[p] >> 8) & 0xFF; /* red and half green  */
        uint16_t gb = frameBuffer[p] & 0xFF;        /* half green and blue */

        *(volatile uint8_t*)LCD_FSMC_ADDR_DATA = ((uint8_t) rg);
        *(volatile uint8_t*)LCD_FSMC_ADDR_DATA = ((uint8_t) gb);
        
//         /* Send red and half green */
//         GPIOD->BSRRH = 0xC023;                      /* Clear D0, D1, D2, D3, WR */
//         GPIOE->BSRRH = 0x0780;                      /* Clear D4, D5, D6, D7 */
//         GPIOD->BSRRL = ((rg << 14) & 0xC000)        /* Set D0, D1 */
//                         | ((rg >> 2) & 0x0003);     /* D2, D3 */
//         GPIOE->BSRRL = (rg << 3) & 0x0780;          /* Set D4, D5, D6, D7 */
//         delayUs(RENDER_WR_SETUP_DELAY);
//         GPIOD->BSRRL = (1 << 5);                    /* Set WR line */
//         delayUs(RENDER_NEXT_BYTE_DELAY);
// 
//         /* Send remaining half green and blue */
//         GPIOD->BSRRH = 0xC023;                      /* Clear D0, D1, D2, D3, WR */
//         GPIOE->BSRRH = 0x0780;                      /* Clear D4, D5, D6, D7 */
//         GPIOD->BSRRL = ((gb << 14) & 0xC000)        /* Set D0, D1 */
//                         | ((gb >> 2) & 0x0003);     /* D2, D3 */
//         GPIOE->BSRRL = (gb << 3) & 0x0780;          /* Set D4, D5, D6, D7 */
//         delayUs(RENDER_WR_SETUP_DELAY);
//         GPIOD->BSRRL = (1 << 5);                    /* Set WR line */
//         delayUs(RENDER_NEXT_BYTE_DELAY);
    }

    gpio_setPin(CS);
}

uint16_t *lcd_getFrameBuffer()
{
    return frameBuffer;
}
