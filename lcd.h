#ifndef _LCD_H_
#define _LCD_H_

typedef struct lcd
{
	int rs;
	int rw;
	int e;
	int data[8];
	int last_addr;
} lcd_t;

int lcd_init(const lcd_t *lcd);
int lcd_setup(const lcd_t *lcd);
int lcd_display_on(const lcd_t *lcd, int display_on, int cusror_on, int blink_on);
int lcd_clear(const lcd_t *lcd);
int lcd_entry_mode(const lcd_t *lcd, int increment_mode, int entire_shift);
int lcd_return_home(const lcd_t *lcd);
int lcd_set_ddram_addr(const lcd_t *lcd, int addr);
int lcd_set_cgram_addr(const lcd_t *lcd, int addr);
int lcd_get_addr(const lcd_t *lcd);
int lcd_write_data(const lcd_t *lcd, int addr);

#endif
