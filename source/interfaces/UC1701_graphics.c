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

#include "graphics.h"

INLINE void graphicsInit(uint16_t backgroundColor)
{
#if ! defined(PLATFORM_GD77S)
	GPIO_PinWrite(GPIO_Display_CS, Pin_Display_CS, 0);// Enable CS permanently
    // Set the LCD parameters...
	UC1701_setCommandMode();
	UC1701_transfer(0xE2); // System Reset
	UC1701_transfer(0x2F); // Voltage Follower On
	UC1701_transfer(0x81); // Set Electronic Volume = 15
	UC1701_transfer(nonVolatileSettings.displayContrast); //
	UC1701_transfer(0xA2); // Set Bias = 1/9
#if defined(PLATFORM_RD5R)
	UC1701_transfer(0xA0); // Set SEG Direction
	UC1701_transfer(0xC8); // Set COM Direction
#else
	UC1701_transfer(0xA1); // A0 Set SEG Direction
	UC1701_transfer(0xC0); // Set COM Direction
#endif
	if (backgroundColor == COLOR_BLACK)
	{
		UC1701_transfer(0xA7); // Black background, white pixels
	}
	else
	{
		UC1701_transfer(0xA4); // White background, black pixels
	}

    UC1701_setCommandMode();
    UC1701_transfer(0xAF); // Set Display Enable
    ucClearBuf();
    ucRender();
#endif // ! PLATFORM_GD77S
}

INLINE void clearBuf(void);
INLINE void clearRows(int16_t startRow, int16_t endRow, uint16_t backgroundColor)
{
	// Boundaries
	if (((startRow < 0) || (endRow < 0)) || ((startRow > 8) || (endRow > 8)) || (startRow == endRow))
		return;

	if (endRow < startRow)
	{
		swap(startRow, endRow);
	}

	// memset would be faster than ucFillRect
	//ucFillRect(0, (startRow * 8), 128, (8 * (endRow - startRow)), true);
    memset(screenBuf + (128 * startRow), (backgroundColor == COLOR_BLACK ? 0xFF : 0x00), (128 * (endRow - startRow)));
}
INLINE void render(void);
INLINE void renderRows(int16_t startRow, int16_t endRow);
INLINE void printCentered(uint8_t y, const  char *text, font_t fontSize);
INLINE void printAt(uint8_t x, uint8_t y,const  char *text, font_t fontSize);
INLINE int printCore(int16_t x, int16_t y,const char *szMsg, font_t fontSize, textAlign_t alignment, uint16_t color);

INLINE int16_t setPixel(int16_t x, int16_t y, uint16_t color)
{
	int16_t i;

	i = ((y >> 3) << 7) + x;
	if (i < 0 || i > 1023)
	{
		return -1;// off the screen
	}

	if (color)
	{
		screenBuf[i] |= (0x1 << (y & 7));
	}
	else
	{
		screenBuf[i] &= ~(0x1 << (y & 7));
	}
	return 0;
}

INLINE void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
INLINE void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
INLINE void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

INLINE void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
INLINE void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

INLINE void drawEllipse(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

INLINE void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
INLINE void fillTriangle ( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

INLINE void fillArc(uint16_t x, uint16_t y, uint16_t radius, uint16_t thickness, float start, float end, uint16_t color);

INLINE void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
INLINE void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
INLINE void drawRoundRectWithDropShadow(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

INLINE void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
INLINE void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, bool isInverted);
INLINE void drawRectWithDropShadow(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

INLINE void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
INLINE void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);

INLINE void setContrast(uint8_t contrast);
INLINE void setInverseVideo(bool isInverted);

INLINE void drawChoice(choice_t choice, bool clearRegion);

INLINE void * getDisplayBuffer(void);

#endif /* DISPLAY_H */
