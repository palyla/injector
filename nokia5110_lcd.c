#include <avr-nokia5110/nokia5110.h>


void lcd_init(void) {
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("Volvo", 1);
    nokia_lcd_set_cursor(0, 10);
    nokia_lcd_write_string("- one love!", 1);
    nokia_lcd_set_cursor(0, 20);
    nokia_lcd_render();
}

void lcd_set_cursor(int x, int y) {
	nokia_lcd_set_cursor(x, y);
}

void lcd_write_string(const char *str) {
	nokia_lcd_write_string(str, 1);
}


void lcd_clear(void) {
	nokia_lcd_clear();
}

void lcd_render(void) {
	nokia_lcd_render();
}
