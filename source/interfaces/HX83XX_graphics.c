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

#include <string.h>
#include <stdlib.h>
#include "graphics.h"
#include "lcd.h"

#define INLINE __attribute__((always_inline)) inline
#define LE2BE(x) __builtin_bswap16(x)
#define swap(x, y) do { typeof(x) t = x; x = y; y = t; } while(0)

#define FB_SIZE SCREEN_WIDTH * SCREEN_HEIGTH * 2
#define COLOR_DEPTH 16
#define N_ROWS 8
#define ROW_HEIGHT 16

uint16_t *screenBuf;

INLINE void graphicsInit(uint16_t backgroundColor)
{
    lcd_init();
    screenBuf = lcd_getFrameBuffer();
    clearBuf();
    render();
}

INLINE void clearBuf(void)
{
	memset(screenBuf, 0x00, FB_SIZE);
}

INLINE void clearRows(int16_t startRow, int16_t endRow, uint16_t backgroundColor)
{
	// Boundaries
	if (((startRow < 0) || (endRow < 0)) || ((startRow > 8) || (endRow > 8)) || (startRow == endRow))
		return;

	if (endRow < startRow)
	{
		swap(startRow, endRow);
	}

    for(uint16_t i = startRow * ROW_HEIGHT * SCREEN_WIDTH;
        i < endRow * ROW_HEIGHT * SCREEN_WIDTH; i++)
        screenBuf[i] = backgroundColor;
}

INLINE void render(void)
{
	renderRows(0,N_ROWS);
}

INLINE void renderRows(int16_t startRow, int16_t endRow)
{
    // TODO: implement split framebuffer rendering
    lcd_render();
}

//INLINE void printCentered(uint8_t y, const  char *text, font_t fontSize);
//INLINE void printAt(uint8_t x, uint8_t y,const  char *text, font_t fontSize);
//INLINE int printCore(int16_t x, int16_t y,const char *szMsg, font_t fontSize, textAlign_t alignment, bool isInverted);

INLINE int16_t setPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGTH)
		return -1; // off the screen

    // Framebuffer is Big Endian, fix endianness before writing
    screenBuf[x+y*SCREEN_WIDTH] = LE2BE(color);
	return 0;
}

INLINE void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep)
	{
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1)
	{
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1)
		ystep = 1;
	else
		ystep = -1;

	for (; x0<=x1; x0++)
	{
		if (steep)
			setPixel(y0, x0, color);
		else
			setPixel(x0, y0, color);

		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}

INLINE void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	fillRect(x, y, 1, h, color);
}

INLINE void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	fillRect(x, y, w, 1, color);
}

//INLINE void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
//INLINE void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
//
//INLINE void drawEllipse(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
//
//INLINE void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
//INLINE void fillTriangle ( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
//
//INLINE void fillArc(uint16_t x, uint16_t y, uint16_t radius, uint16_t thickness, float start, float end, uint16_t color);
//
//INLINE void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
//INLINE void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
//INLINE void drawRoundRectWithDropShadow(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

INLINE void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	drawFastHLine(x        , y        , w, color);
	drawFastHLine(x        , y + h - 1, w, color);
	drawFastVLine(x        , y        , h, color);
	drawFastVLine(x + w - 1, y        , h, color);
}

INLINE void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
{
    for(uint16_t vscan = y; vscan < y+height; vscan++) {
        for(uint16_t hscan = x; hscan < x+width; hscan++) {
            screenBuf[vscan * SCREEN_WIDTH + hscan] = LE2BE(color);
        }
    }
}

//INLINE void drawRectWithDropShadow(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
//
//INLINE void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
//INLINE void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
//
//INLINE void setContrast(uint8_t contrast);
//INLINE void setInverseVideo(bool isInverted);
//
//INLINE void drawChoice(choice_t choice, bool clearRegion);

INLINE void *getDisplayBuffer(void)
{
    return screenBuf;
}
