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

#include "gpio.h"
#include "lcd.h"
#include <stddef.h>

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

/*
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

// Extended command set
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

static void lcd_writeCmd(uint8_t cmd)
{
    gpio_clearPin(RS);
    gpio_setPin(RD);
    gpio_clearPin(WR);
    (cmd & 0x01) ? gpio_setPin(D0) : gpio_clearPin(D0);
    (cmd & 0x02) ? gpio_setPin(D1) : gpio_clearPin(D1);
    (cmd & 0x04) ? gpio_setPin(D2) : gpio_clearPin(D2);
    (cmd & 0x08) ? gpio_setPin(D3) : gpio_clearPin(D3);
    (cmd & 0x10) ? gpio_setPin(D4) : gpio_clearPin(D4);
    (cmd & 0x20) ? gpio_setPin(D5) : gpio_clearPin(D5);
    (cmd & 0x40) ? gpio_setPin(D6) : gpio_clearPin(D6);
    (cmd & 0x80) ? gpio_setPin(D7) : gpio_clearPin(D7);
    uDelay(20);
    gpio_setPin(WR);
}

static void lcd_writeData(uint8_t val)
{
    gpio_setPin(RS);
    gpio_setPin(RD);
    gpio_clearPin(WR);
    (val & 0x01) ? gpio_setPin(D0) : gpio_clearPin(D0);
    (val & 0x02) ? gpio_setPin(D1) : gpio_clearPin(D1);
    (val & 0x04) ? gpio_setPin(D2) : gpio_clearPin(D2);
    (val & 0x08) ? gpio_setPin(D3) : gpio_clearPin(D3);
    (val & 0x10) ? gpio_setPin(D4) : gpio_clearPin(D4);
    (val & 0x20) ? gpio_setPin(D5) : gpio_clearPin(D5);
    (val & 0x40) ? gpio_setPin(D6) : gpio_clearPin(D6);
    (val & 0x80) ? gpio_setPin(D7) : gpio_clearPin(D7);
    uDelay(20);
    gpio_setPin(WR);
}

void lcd_init()
{
    gpio_setMode(D0,  OUTPUT);
    gpio_setMode(D1,  OUTPUT);
    gpio_setMode(D2,  OUTPUT);
    gpio_setMode(D3,  OUTPUT);
    gpio_setMode(D4,  OUTPUT);
    gpio_setMode(D5,  OUTPUT);
    gpio_setMode(D6,  OUTPUT);
    gpio_setMode(D7,  OUTPUT);
    gpio_setMode(WR,  OUTPUT);
    gpio_setMode(RD,  OUTPUT);
    gpio_setMode(CS,  OUTPUT);
    gpio_setMode(RS,  OUTPUT);
    gpio_setMode(RST, OUTPUT);

    gpio_clearPin(D0);
    gpio_clearPin(D1);
    gpio_clearPin(D2);
    gpio_clearPin(D3);
    gpio_clearPin(D4);
    gpio_clearPin(D5);
    gpio_clearPin(D6);
    gpio_clearPin(D7);

    gpio_setPin(WR);    /* Idle state is high level, for these */
    gpio_setPin(RD);
    gpio_setPin(CS);
    gpio_setPin(RS);

    gpio_clearPin(RST); /* Put LCD in reset mode */

    uDelay(20000);
    gpio_setPin(RST);   /* Exit from reset */

    gpio_clearPin(CS);
    lcd_writeCmd(CMD_SLPOUT);
    uDelay(120*1000);
    lcd_writeCmd(CMD_NORON);
    lcd_writeCmd(CMD_INVOFF);
    lcd_writeCmd(CMD_DISPON);
    gpio_setPin(CS);
}

void lcd_render()
{
    gpio_clearPin(CS);
    lcd_writeCmd(CMD_RAMWR);

    for(size_t i = 0; i < 168*128; i++)
    {
        /* Send random values to LCD framebuffer */
        static long int a = 16807L;
        static long int m = 2147483647L;
        static long int q = 127773L;
        static long int r = 2836L;

        long int lo, hi, test, seed;

        seed = i;
        hi = seed / q;
        lo = seed % q;
        test = a * lo - r * hi;
        if (test > 0) seed = test;
        else seed = test + m;

        lcd_writeData(seed & 0xFF);
        lcd_writeData((seed >> 8) & 0xFF);
    }
    gpio_setPin(CS);
}
