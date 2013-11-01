#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include "lcd.h"

#define COMMAND	0
#define DATA	1

static void lcd_write(const lcd_t *lcd, int is_data, int data);

int lcd_init(const lcd_t *lcd)
{
	wiringPiSetup();

	pinMode(lcd->rs, OUTPUT);
	pinMode(lcd->rw, OUTPUT);
	pinMode(lcd->e,  OUTPUT);

	usleep(30000);
	return 0;
}

int lcd_setup(const lcd_t *lcd)
{
	lcd_write(lcd, COMMAND, 0x28); // function set, display=off, 1-line mode
	usleep(40);
	lcd_write(lcd, COMMAND, 0x0F); // display on, cursor on, blink on
	usleep(40);
	lcd_write(lcd, COMMAND, 0x01); // display clear
	usleep(150);
	lcd_write(lcd, COMMAND, 0x06); // entry mode set, increment, entry shift off
	usleep(40);
	return 0;
}

int lcd_display_on(const lcd_t *lcd, int display_on, int cursor_on, int blink_on)
{
	lcd_write(lcd, COMMAND,
			0x08 | (display_on?0x04:0) | (cursor_on?0x02:0) | (blink_on?0x01:0));
	return 0;
}

int lcd_set_ddram_addr(const lcd_t *lcd, int addr)
{
	lcd_write(lcd, COMMAND, 0x80 | (addr & 0x7f));
	return 0;
}

int lcd_set_cgram_addr(const lcd_t *lcd, int addr)
{
	lcd_write(lcd, COMMAND, 0x40 | (addr & 0x3f));
	return 0;
}

int lcd_entry_mode(const lcd_t *lcd, int increment_mode, int entire_shift)
{
	lcd_write(lcd, COMMAND, 0x04 | (increment_mode?0x02:0) | (entire_shift?0x01:0));
	return 0;
}

int lcd_clear(const lcd_t *lcd)
{
	lcd_write(lcd, COMMAND, 0x01);
	return 0;
}

int lcd_get_addr(const lcd_t *lcd)
{
	return lcd->last_addr;
}

int lcd_write_data(const lcd_t *lcd, int data)
{
	lcd_write(lcd, DATA, data);
	return 0;
}

static void lcd_write(const lcd_t *lcd, int is_data, int data)
{
	int i;
	int busy;
	int addr;

	if(is_data)
		printf("data %02x\n", data);
	else
		printf("command %02x\n", data);

	for(i = 4; i < 8; ++i)pinMode(lcd->data[i], OUTPUT);

	digitalWrite(lcd->rs, is_data);
	digitalWrite(lcd->rw, 0);

	digitalWrite(lcd->data[4], data & 0x10);
	digitalWrite(lcd->data[5], data & 0x20);
	digitalWrite(lcd->data[6], data & 0x40);
	digitalWrite(lcd->data[7], data & 0x80);

	digitalWrite(lcd->e,  1);
	usleep(40);
	digitalWrite(lcd->e,  0);

	if(!is_data && (data & 0xE0) == 0x20){
		// function set
		printf("send it twice?\n");

		digitalWrite(lcd->data[4], data & 0x10);
		digitalWrite(lcd->data[5], data & 0x20);
		digitalWrite(lcd->data[6], data & 0x40);
		digitalWrite(lcd->data[7], data & 0x80);

		digitalWrite(lcd->e,  1);
		usleep(40);
		digitalWrite(lcd->e,  0);
	}

	//usleep(40);
	digitalWrite(lcd->data[4], data & 0x01);
	digitalWrite(lcd->data[5], data & 0x02);
	digitalWrite(lcd->data[6], data & 0x04);
	digitalWrite(lcd->data[7], data & 0x08);
	digitalWrite(lcd->e,  1);
	usleep(40);
	digitalWrite(lcd->e,  0);

	digitalWrite(lcd->rw, 1);
	digitalWrite(lcd->rs, 0);

	for(i = 4; i < 8; ++i)pinMode(lcd->data[i], INPUT);

	do{
		digitalWrite(lcd->e, 1);
		usleep(40);
		busy = digitalRead(lcd->data[7]);
		addr =
				(digitalRead(lcd->data[6]) << 4) |
				(digitalRead(lcd->data[5]) << 3) |
				(digitalRead(lcd->data[4]) << 2);
		digitalWrite(lcd->e, 0);
	}while(busy);
	if(addr != 0)printf("addr = %02x\n", addr);
}
