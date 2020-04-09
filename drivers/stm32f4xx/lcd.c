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

// LCD framebuffer, allocated on the heap by lcd_init().
// Pixel format is RGB565, 16 bit per pixel
static uint16_t *frameBuffer;

static void uDelay (const uint32_t usec)
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

static void setDataLines(uint8_t x)
{
    /* Clear all data lines */
    GPIOD->BSRRH = 0xC003;
    GPIOE->BSRRH = 0x0780;

    uint16_t xx = x;
    GPIOD->BSRRL = ((xx << 14) & 0xC000) | ((xx >> 2) & 0x0003);
    GPIOE->BSRRL = (xx << 3) & 0x0780;
}

static uint8_t getDataLines()
{
    uint8_t val = 0;
    val = (GPIOE->IDR & 0x0780) >> 3;       /* High nibble */
    val |= ((GPIOE->IDR & 0xC000) >> 14)    /* Low nibble */
        |  ((GPIOE->IDR & 0x0003) << 2);
    return val;
}

#define LCD_FSMC_ADDR_COMMAND 0x60000000
#define LCD_FSMC_ADDR_DATA    0x60040000

__attribute__ ((noinline)) static void writeCmd(uint8_t cmd)
{
    *(volatile uint8_t*)LCD_FSMC_ADDR_COMMAND = cmd;

//     gpio_clearPin(RS);
//     gpio_setPin(RD);
//     gpio_clearPin(WR);
//     setDataLines(cmd);
//     uDelay(100);
//     gpio_setPin(WR);
//     uDelay(100);
}
__attribute__ ((noinline)) static void writeData(uint8_t val)
{
    *(volatile uint8_t*)LCD_FSMC_ADDR_DATA = val;

//     gpio_setPin(RS);
//     gpio_setPin(RD);
//     gpio_clearPin(WR);
//     setDataLines(val);
//     uDelay(100);
//     gpio_setPin(WR);
//     uDelay(100);
}

static uint8_t lcd_readReg(uint8_t reg)
{
    writeCmd(reg);

    gpio_clearPin(RD);
    gpio_setPin(RS);

    gpio_setMode(D0, INPUT);
    gpio_setMode(D1, INPUT);
    gpio_setMode(D2, INPUT);
    gpio_setMode(D3, INPUT);
    gpio_setMode(D4, INPUT);
    gpio_setMode(D5, INPUT);
    gpio_setMode(D6, INPUT);
    gpio_setMode(D7, INPUT);

    uDelay(100);
    gpio_setPin(RD);
    uDelay(100);
    uint8_t dummy = getDataLines();

    gpio_clearPin(RD);
    uDelay(100);
    gpio_setPin(RD);
    uDelay(100);
    uint8_t value = getDataLines();

    gpio_setMode(D0, OUTPUT);
    gpio_setMode(D1, OUTPUT);
    gpio_setMode(D2, OUTPUT);
    gpio_setMode(D3, OUTPUT);
    gpio_setMode(D4, OUTPUT);
    gpio_setMode(D5, OUTPUT);
    gpio_setMode(D6, OUTPUT);
    gpio_setMode(D7, OUTPUT);

    printf("Dummy %d, value %d\r\n", dummy, value);

    return 0;
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
                | TIM_CCMR1_OC1M_1
                | TIM_CCMR1_OC1PE;
    TIM8->CCER  |= TIM_CCER_CC1E;
    TIM8->BDTR  |= TIM_BDTR_MOE;
    TIM8->CCR1 = 0;
    TIM8->EGR  = TIM_EGR_UG;        /* Update registers */
    TIM8->CR1 |= TIM_CR1_CEN;       /* Start timer */

    /* Configure GPIO, TIM8 is on AF3 */
    gpio_setMode(GPIOC, 6, ALTERNATE);
    gpio_setAlternateFunction(GPIOC, 6, 3);

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
     * - FACCEN    = 0: NOR flash memory disabled
     * - MWID      = 0: 8 bit external memory device
     * - MTYP      = 0: SRAM
     * - MUXEN     = 0: addr/data not multiplexed
     * - MBNEN     = 1: enable bank
     */
    RCC->AHB3ENR |= RCC_AHB3ENR_FSMCEN;
    FSMC_Bank1->BTCR[0] = 0x145b;
                        //= FSMC_BCR1_EXTMOD
                        //| FSMC_BCR1_WREN
                        //| FSMC_BCR1_MBKEN;

    /* BTR1 config:
     * - ACCMOD  = 1: access mode B
     * - DATLAT  = 0: don't care in asynchronous mode
     * - CLKDIV  = 2: FSMC_CLK period = 3xHCLK period -> 3*5.95ns = 17.85ns
     * - BUSTURN = 9: time between two consecutive write accesses: (1 + 2 + BUSTURN)*HCLK_period = 71.4ns > twc (66ns)
     * - DATAST  = 3: we must have LCD twrl < DATAST*HCLK_period: 15ns < 3*5.95 = 17.85ns
     * - ADDHLD  = 1: used only in mode D
     * - ADDSET  = 1: address setup time 3*HCLK_period = 17.85ns
     */
    FSMC_Bank1->BTCR[1] = (1 << 28) /* ACCMOD */
                        | (5 << 20) /* CLKDIV */
                        | (0 << 16) /* BUSTURN */
                        | (5 << 8)  /* DATAST */
                        | (7 << 4)  /* ADDHLD */
                        | 7;        /* ADDSET */

//     /* For BWTR set the same values as per BTR1 */
//     FSMC_Bank1E->BWTR[0] = (1 << 28) /* ACCMOD */
//                          | (9 << 16) /* BUSTURN */
//                          | (3 << 8)  /* DATAST */
//                          | (1 << 4)  /* ADDHLD */
//                          | 3;        /* ADDSET */ 

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

    uDelay(20000);
    gpio_setPin(RST);   /* Exit from reset */

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

    gpio_setMode(D0, ALTERNATE);
    gpio_setMode(D1, ALTERNATE);
    gpio_setMode(D2, ALTERNATE);
    gpio_setMode(D3, ALTERNATE);
    gpio_setMode(D4, ALTERNATE);
    gpio_setMode(D5, ALTERNATE);
    gpio_setMode(D6, ALTERNATE);
    gpio_setMode(D7, ALTERNATE);
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
    gpio_setAlternateFunction(WR, 12);
    gpio_setAlternateFunction(RD, 12);
    
    gpio_clearPin(CS);
    writeCmd(CMD_SLPOUT);
    uDelay(120*1000);
    writeCmd(CMD_GAMSET);
    writeData(0x04);
    writeCmd(CMD_SETPWCTR);
    writeData(0x0A);
    writeData(0x14);
    writeCmd(CMD_SETSTBA);
    writeData(0x0A);
    writeData(0x00);
    writeCmd(CMD_COLMOD);
    writeData(0x05);
    uDelay(10*1000);
//     writeCmd(CMD_CASET);
//     writeData(0x00);
//     writeData(0x00);
//     writeData(0x00);
//     writeData(0x9E);
//     writeCmd(CMD_RASET);
//     writeData(0x00);
//     writeData(0x00);
//     writeData(0x00);
//     writeData(0x79);
    writeCmd(CMD_NORON);
    uDelay(10*1000);
    writeCmd(CMD_DISPON);
    uDelay(120*1000);
    writeCmd(CMD_RAMWR);

    gpio_setPin(CS);

    uDelay(120*1000);
    gpio_clearPin(CS);
    lcd_readReg(CMD_RDDIDIF);
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
//     gpio_setMode(D0, OUTPUT);
//     gpio_setMode(D1, OUTPUT);
//     gpio_setMode(D2, OUTPUT);
//     gpio_setMode(D3, OUTPUT);
//     gpio_setMode(D4, OUTPUT);
//     gpio_setMode(D5, OUTPUT);
//     gpio_setMode(D6, OUTPUT);
//     gpio_setMode(D7, OUTPUT);
//     gpio_setMode(WR, OUTPUT);
//     gpio_setMode(RD, OUTPUT);

    writeCmd(CMD_RAMWR);

    /* Now try writing to framebuffer using FSMC peripheral */
//     gpio_setMode(D0, ALTERNATE);
//     gpio_setMode(D1, ALTERNATE);
//     gpio_setMode(D2, ALTERNATE);
//     gpio_setMode(D3, ALTERNATE);
//     gpio_setMode(D4, ALTERNATE);
//     gpio_setMode(D5, ALTERNATE);
//     gpio_setMode(D6, ALTERNATE);
//     gpio_setMode(D7, ALTERNATE);
//     gpio_setMode(WR, ALTERNATE);
//     gpio_setMode(RD, ALTERNATE);
// 
//     gpio_setAlternateFunction(D0, 12);
//     gpio_setAlternateFunction(D1, 12);
//     gpio_setAlternateFunction(D2, 12);
//     gpio_setAlternateFunction(D3, 12);
//     gpio_setAlternateFunction(D4, 12);
//     gpio_setAlternateFunction(D5, 12);
//     gpio_setAlternateFunction(D6, 12);
//     gpio_setAlternateFunction(D7, 12);
//     gpio_setAlternateFunction(WR, 12);
//     gpio_setAlternateFunction(RD, 12);

    
    for(uint8_t r = 0; r < 128; r++)
    {
        for(uint8_t c = 0; c < 160; c++)
        {
            volatile uint32_t *p = ((uint32_t *)0x60000000);
            *p = 2*r;
//             writeData(2*r);
//             writeData((i >> 8) & 0xFF);
        }
    }
    gpio_setPin(CS);
}
