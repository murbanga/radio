
CC=arm-unknown-linux-gnueabi-gcc
#/home/waev/x-tools/arm-unknown-linux-gnueabi/bin/arm-unknown-linux-gnueabi-gcc
INCLUDES = ../wiringPi/wiringPi
LIBDIR   = ../wiringPi/wiringPi
CFLAGS  += -O2 -I${INCLUDES} -Wall -mfloat-abi=softfp
LDFLAGS += -L${LIBDIR} -lwiringPi -lpthread
# -lmpdclient

volume.o: volume.c
lcd.o: lcd.c lcd.h
lcd_main.o: lcd_main.c lcd.h

volume: volume.o
	${CC} $< -o $@ ${LDFLAGS}

lcd: lcd_main.o lcd.o
	${CC} lcd_main.o lcd.o -o $@ ${LDFLAGS}

all: volume lcd
	cp volume /tmp
	cp lcd /tmp
	
clean:
	rm *.o
	rm volume
	rm lcd
