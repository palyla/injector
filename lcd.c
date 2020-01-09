#include "lcd.h"

/* static unsigned char fontsize = 1; */


int lcd_putchar(char c, FILE *stream) {
#ifdef T6963C_LCD
	GLCD_WriteChar(c);
#endif /* T6963C_LCD */

#ifdef NOKIA_5110_LCD
	nokia_lcd_write_char(str, 1);
#endif /* NOKIA_5110_LCD */

}


void lcd_clear(void) {
	
#ifdef T6963C_LCD
	GLCD_ClearText();
#endif /* T6963C_LCD */

#ifdef NOKIA_5110_LCD
	nokia_lcd_clear();
#endif /* NOKIA_5110_LCD */

}


void lcd_init(void) {

#ifdef T6963C_LCD
	GLCD_Initalize();
#endif /* T6963C_LCD */

#ifdef NOKIA_5110_LCD
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_render();
#endif /* NOKIA_5110_LCD */
}


void lcd_render(void) {

#ifdef T6963C_LCD
	/* This display rendering automatically */
#endif /* T6963C_LCD */

#ifdef NOKIA_5110_LCD
	nokia_lcd_render();
#endif /* NOKIA_5110_LCD */

}


void lcd_goto(unsigned char x, unsigned char y) {

#ifdef T6963C_LCD
	GLCD_TextGoTo(x, y);
#endif /* T6963C_LCD */

#ifdef NOKIA_5110_LCD
	nokia_lcd_set_cursor(x, y);
#endif /* NOKIA_5110_LCD */
}


void lcd_rectangle(unsigned char x, unsigned char y, unsigned char b, unsigned char a) {
    unsigned char j;

    for (j = 0; j < a; j++) {
        lcd_setpixel(x, y + j);
        lcd_setpixel(x + b - 1, y + j);
    }

    for (j = 0; j < b; j++) {
        lcd_setpixel(x + j, y);
        lcd_setpixel(x + j, y + a - 1);
    }
}


void lcd_circle(unsigned char cx, unsigned char cy, unsigned char radius) {
    int x, y, xchange, ychange, radiusError;
    
    x = radius;
    y = 0;
    xchange = 1 - 2 * radius;
    ychange = 1;
    radiusError = 0;
    while (x >= y) {
        lcd_setpixel(cx + x, cy + y);
        lcd_setpixel(cx - x, cy + y);
        lcd_setpixel(cx - x, cy - y);
        lcd_setpixel(cx + x, cy - y);
        lcd_setpixel(cx + y, cy + x);
        lcd_setpixel(cx - y, cy + x);
        lcd_setpixel(cx - y, cy - x);
        lcd_setpixel(cx + y, cy - x);
        y++;
        radiusError += ychange;
        ychange += 2;
        if (2 * radiusError + xchange > 0) {
            x--;
            radiusError += xchange;
            xchange += 2;
        }
    }
}


void lcd_line(int x1, int y1, int x2, int y2) {
    int CurrentX, CurrentY, Xinc, Yinc,
            Dx, Dy, TwoDx, TwoDy,
            TwoDxAccumulatedError, TwoDyAccumulatedError;

    Dx = (x2 - x1);
    Dy = (y2 - y1);

    TwoDx = Dx + Dx;
    TwoDy = Dy + Dy;

    CurrentX = x1;
    CurrentY = y1;

    Xinc = 1;
    Yinc = 1;

    if (Dx < 0) {
        Xinc = -1;
        Dx = -Dx;
        TwoDx = -TwoDx;
    }

    if (Dy < 0) {
        Yinc = -1;
        Dy = -Dy;
        TwoDy = -TwoDy;
    }

    lcd_setpixel(x1, y1);

    if ((Dx != 0) || (Dy != 0)) {

        if (Dy <= Dx) {
            TwoDxAccumulatedError = 0;
            do {
                CurrentX += Xinc;
                TwoDxAccumulatedError += TwoDy;
                if (TwoDxAccumulatedError > Dx) {
                    CurrentY += Yinc;
                    TwoDxAccumulatedError -= TwoDx;
                }
                lcd_setpixel(CurrentX, CurrentY);
            } while (CurrentX != x2);
        } else {
            TwoDyAccumulatedError = 0;
            do {
                CurrentY += Yinc;
                TwoDyAccumulatedError += TwoDx;
                if (TwoDyAccumulatedError > Dy) {
                    CurrentX += Xinc;
                    TwoDyAccumulatedError -= TwoDy;
                }
                lcd_setpixel(CurrentX, CurrentY);
            } while (CurrentY != y2);
        }
    }
}
