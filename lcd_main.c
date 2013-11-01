#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"

#define PIN_RS	13
#define PIN_RW	12
#define PIN_E	5
#define PIN_D4	6
#define PIN_D5	14
#define PIN_D6	10
#define PIN_D7	11

#define COM_ON		0x01
#define COM_OFF		0x02
#define COM_PRINT	0x04
#define COM_INIT	0x08
#define COM_CLEAR	0x10

int hex_digit(int x)
{
	if(x >= '0' && x <= '9')return x - '0';
	if(x >= 'a' && x <= 'f')return 10 + x - 'a';
	if(x >= 'A' && x <= 'F')return 10 + x - 'A';
	return -1;
}

int parse_hex(char *s, const char *p, int maxlen)
{
	int i, j;
	int len = strlen(p);
	for(i = 0; i < len && j < maxlen - 1; i+=2, ++j){
		int hi = hex_digit(p[i]);
		int lo = hex_digit(p[i + 1]);
		if(hi >= 0 && lo >= 0)
			s[j] = hi*16 + lo;
		else{
			printf("wrong digit %c%c\n", p[i], p[i+1]);
			return -1;
		}
	}
	s[i] = 0;
	return i;
}

int main(int argc, char **argv)
{
	int i;
	int command = 0;
	int x = -1;
	int y = -1;
	char string[256] = "";

	lcd_t lcd;

	lcd.rs = PIN_RS;
	lcd.rw = PIN_RW;
	lcd.e  = PIN_E;
	lcd.data[0] = -1;
	lcd.data[1] = -1;
	lcd.data[2] = -1;
	lcd.data[3] = -1;
	lcd.data[4] = PIN_D4;
	lcd.data[5] = PIN_D5;
	lcd.data[6] = PIN_D6;
	lcd.data[7] = PIN_D7;

	for(i = 0; i < argc; ++i){
		if(!strcmp("-init", argv[i]))command |= COM_INIT;
		else if(!strcmp("-on", argv[i]))command |= COM_ON;
		else if(!strcmp("-off", argv[i]))command |= COM_OFF;
		else if(!strcmp("-print", argv[i]))command |= COM_PRINT;
		else if(!strcmp("-clear", argv[i]))command |= COM_CLEAR;
		else if(!strcmp("-x", argv[i]))x = atoi(argv[++i]);
		else if(!strcmp("-y", argv[i]))y = atoi(argv[++i]);
		else if(!strcmp("-s", argv[i]))strncpy(string, argv[++i], sizeof(string));
		else if(!strcmp("-h", argv[i]))parse_hex(string, argv[++i], sizeof(string));
	}

	if(command == 0){
		printf("no commands specified\n");
		return -1;
	}

	lcd_init(&lcd);

	if(command & COM_INIT){
		lcd_setup(&lcd);
	}

	if(command & COM_ON){
		lcd_display_on(&lcd, 1, 1, 1);
	}

	if(command & COM_CLEAR){
		lcd_clear(&lcd);
	}

	if(command & COM_PRINT){
		static const int line_offset[4] = {0x00, 0x28, 0x10, 0x38};
		char *p = string;

		if(x >= 0 && y >= 0){
			int addr = (x & 15) + line_offset[y & 3];
			lcd_set_ddram_addr(&lcd, addr);
		}

		while(*p)lcd_write_data(&lcd, *p++);
	}

	if(command & COM_OFF){
		lcd_display_on(&lcd, 0, 0, 0);
	}

	return 0;
}
