#ifndef __INJECTOR_LCD_H__
#define __INJECTOR_LCD_H__


void lcd_init(void);
void lcd_set_cursor(int x, int y);
void lcd_write_string(const char *str);
void lcd_clear(void);
void lcd_render(void);


#endif /* __INJECTOR_LCD_H__ */