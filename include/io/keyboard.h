/*
 * Copyright (C)2020 Niccolò Izzo, IU2KIN, Silvano Seva IU2KWO
 * Copyright (C)2019 Kai Ludwig, DG4KLU
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _FW_KEYBOARD_H_
#define _FW_KEYBOARD_H_

#include "common.h"

#if defined(PLATFORM_GD77) || defined(PLATFORM_GD77S)

// column lines
#define Port_Key_Col0   PORTC
#define GPIO_Key_Col0 	GPIOC
#define Pin_Key_Col0	0
#define Port_Key_Col1   PORTC
#define GPIO_Key_Col1 	GPIOC
#define Pin_Key_Col1 	1
#define Port_Key_Col2   PORTC
#define GPIO_Key_Col2 	GPIOC
#define Pin_Key_Col2 	2
#define Port_Key_Col3   PORTC
#define GPIO_Key_Col3 	GPIOC
#define Pin_Key_Col3 	3

// row lines
#define Port_Key_Row0   PORTB
#define GPIO_Key_Row0 	GPIOB
#define Pin_Key_Row0	19
#define Port_Key_Row1   PORTB
#define GPIO_Key_Row1 	GPIOB
#define Pin_Key_Row1	20
#define Port_Key_Row2   PORTB
#define GPIO_Key_Row2 	GPIOB
#define Pin_Key_Row2	21
#define Port_Key_Row3   PORTB
#define GPIO_Key_Row3 	GPIOB
#define Pin_Key_Row3	22
#define Port_Key_Row4   PORTB
#define GPIO_Key_Row4 	GPIOB
#define Pin_Key_Row4	23

#define N_Rows          5
#define N_Cols          4

#elif defined(PLATFORM_DM1801)

// column lines
#define Port_Key_Col0   PORTC
#define GPIO_Key_Col0 	GPIOC
#define Pin_Key_Col0	0
#define Port_Key_Col1   PORTC
#define GPIO_Key_Col1 	GPIOC
#define Pin_Key_Col1 	1
#define Port_Key_Col2   PORTC
#define GPIO_Key_Col2 	GPIOC
#define Pin_Key_Col2 	2
#define Port_Key_Col3   PORTC
#define GPIO_Key_Col3 	GPIOC
#define Pin_Key_Col3 	3

// row lines
#define Port_Key_Row0   PORTB
#define GPIO_Key_Row0 	GPIOB
#define Pin_Key_Row0	19
#define Port_Key_Row1   PORTB
#define GPIO_Key_Row1 	GPIOB
#define Pin_Key_Row1	20
#define Port_Key_Row2   PORTB
#define GPIO_Key_Row2 	GPIOB
#define Pin_Key_Row2	21
#define Port_Key_Row3   PORTB
#define GPIO_Key_Row3 	GPIOB
#define Pin_Key_Row3	22
#define Port_Key_Row4   PORTB
#define GPIO_Key_Row4 	GPIOB
#define Pin_Key_Row4	23

#define N_Rows          5
#define N_Cols          4

#elif defined(PLATFORM_MD380)

// column lines (LCD_D[0-7])
#define GPIO_Key_Col0_3   GPIOD
#define Mask_Key_Col0_1   0b1100000000000000
#define Offset_Key_Col0_1 14
#define Mask_Key_Col2_3   0b0000000000000011
#define Offset_Key_Col2_3 2
#define GPIO_Key_Col4_7   GPIOE
#define Mask_Key_Col4_7   0b0000011110000000
#define Offset_Key_Col4_7 3

#define GPIO_Key_Col0 	GPIOD
#define Pin_Key_Col0	14
#define GPIO_Key_Col1 	GPIOD
#define Pin_Key_Col1 	15
#define GPIO_Key_Col2 	GPIOD
#define Pin_Key_Col2 	0
#define GPIO_Key_Col3 	GPIOD
#define Pin_Key_Col3 	1
#define GPIO_Key_Col4 	GPIOE
#define Pin_Key_Col4	7
#define GPIO_Key_Col5 	GPIOE
#define Pin_Key_Col5 	8
#define GPIO_Key_Col6 	GPIOE
#define Pin_Key_Col6 	9
#define GPIO_Key_Col7 	GPIOE
#define Pin_Key_Col7 	10

// row lines (K1, K2, K3)
#define GPIO_Key_Row0 	GPIOA
#define Pin_Key_Row0	6
#define GPIO_Key_Row1 	GPIOD
#define Pin_Key_Row1	2
#define GPIO_Key_Row2 	GPIOD
#define Pin_Key_Row2	3

#define N_Rows          2
#define N_Cols          8

#endif




#define SCAN_UP     0x00000100
#define SCAN_DOWN   0x00002000
#define SCAN_LEFT   0x00000200
#define SCAN_RIGHT  0x00000010
#define SCAN_GREEN  0x00000008
#define SCAN_RED    0x00040000
#define SCAN_0      0x00010000
#define SCAN_1      0x00000001
#define SCAN_2      0x00000002
#define SCAN_3      0x00000004
#define SCAN_4      0x00000020
#define SCAN_5      0x00000040
#define SCAN_6      0x00000080
#define SCAN_7      0x00000400
#define SCAN_8      0x00000800
#define SCAN_9      0x00001000
#define SCAN_STAR   0x00008000
#define SCAN_HASH   0x00020000

#define KEY_GREENSTAR   '+'    // GREEN + STAR

#define KEY_UP           1
#define KEY_DOWN         2
#define KEY_LEFT         3
#define KEY_RIGHT        4

#if defined(PLATFORM_DM1801)
#define KEY_VFO_MR       5
#define KEY_A_B          6
#endif

#define KEY_GREEN       13
#define KEY_RED         27
#define KEY_0           '0'
#define KEY_1           '1'
#define KEY_2           '2'
#define KEY_3           '3'
#define KEY_4           '4'
#define KEY_5           '5'
#define KEY_6           '6'
#define KEY_7           '7'
#define KEY_8           '8'
#define KEY_9           '9'
#define KEY_STAR        '*'
#define KEY_HASH        '#'


#define KEY_MOD_DOWN    0x01
#define KEY_MOD_UP      0x02
#define KEY_MOD_LONG    0x04
#define KEY_MOD_PRESS   0x08
#define KEY_MOD_PREVIEW 0x10

#define EVENT_KEY_NONE   0
#define EVENT_KEY_CHANGE 1

#define KEY_DEBOUNCE_COUNTER   20

//#define KEYCHECK(keys,k) (((keys) & 0xffffff) == (k))
//#define KEYCHECK_KEYMOD(keys, k, mask, mod) (((((keys) & 0xffffff) == (k)) && ((keys) & (mask)) == (mod)))
//#define KEYCHECK_MOD(keys, mask, mod) (((keys) & (mask)) == (mod))

#define KEYCHECK_UP(keys, k)       ((keys.key == k) && ((keys.event & KEY_MOD_UP) == KEY_MOD_UP))
#define KEYCHECK_SHORTUP(keys, k)  ((keys.key == k) && ((keys.event & (KEY_MOD_UP | KEY_MOD_LONG)) == KEY_MOD_UP))
#define KEYCHECK_DOWN(keys, k)     ((keys.key == k) && ((keys.event & KEY_MOD_DOWN) == KEY_MOD_DOWN))
#define KEYCHECK_PRESS(keys, k)    ((keys.key == k) && ((keys.event & KEY_MOD_PRESS) == KEY_MOD_PRESS))
#define KEYCHECK_LONGDOWN(keys, k) ((keys.key == k) && ((keys.event & (KEY_MOD_DOWN | KEY_MOD_LONG)) == (KEY_MOD_DOWN | KEY_MOD_LONG)))


//#define KEYCHAR(keys)              ((char)(keys & 0xff))

extern volatile bool keypadLocked;
extern volatile bool keypadAlphaEnable;

typedef struct keyboardCode {
		uint8_t event;
		char key;
} keyboardCode_t;

#define NO_KEYCODE  { .event = 0, .key = 0 }

void fw_init_keyboard(void);
void fw_reset_keyboard(void);
uint8_t fw_read_keyboard_col(void);
uint32_t fw_read_keyboard(void);
void fw_check_key_event(keyboardCode_t *keys, int *event);
bool fw_scan_key(uint32_t scancode, char *keycode);

#endif /* _FW_KEYBOARD_H_ */
