#ifndef __INJECTOR_LCD_H__
#define __INJECTOR_LCD_H__

#include <avr/io.h>
#include <stdio.h>

#ifdef T6963C_LCD
	#include <T6963C.h>
#endif /* T6963C_LCD */

#ifdef NOKIA_5110_LCD
	#include <nokia5110.h>
#endif /* NOKIA_5110_LCD */

extern FILE lcdout;

void lcd_rectangle(unsigned char x, unsigned char y, unsigned char b, unsigned char a);
void lcd_circle(unsigned char cx, unsigned char cy, unsigned char radius);
void lcd_line(int x1, int y1, int x2, int y2);
void lcd_setpixel(unsigned char x, unsigned char y);

void lcd_goto(unsigned char x, unsigned char y);
int lcd_putchar(char c, FILE *stream);
void lcd_clear(void);
void lcd_render(void);
void lcd_init(void);



#endif /* __INJECTOR_LCD_H__ */
