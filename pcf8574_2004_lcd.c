#include <avr/interrupt.h>

#include <lcd1602/lcd1602.h>


void lcd_init(void) {
    lcd1602_init();
    lcd1602_clear();
    lcd1602_goto_xy(1,0);
    lcd1602_send_string("Hello, world!");

}

void lcd_set_cursor(int x, int y) {

}

void lcd_write_string(const char *str){

}

void lcd_clear(void) {

}

void lcd_render(void) {

}

