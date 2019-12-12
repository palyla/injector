DEVICE     = atmega328p
CLOCK      = 16000000
AVRDUDE = avrdude -C /etc/avrdude.conf -c arduino -P /dev/ttyUSB0 -b 57600 -D -p $(DEVICE)

LIBS_H = -Ilib
OBJECTS = lib/lcd1602/i2c.o lib/lcd1602/lcd1602.o nokia5110_lcd.o lib/avr-nokia5110/nokia5110.o uart.o app.o
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)  -Llib -Wl,-u,vfprintf -lprintf_flt -lm


all:	app.hex

.c.o:
	$(COMPILE) $(LIBS_H) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:app.hex:i

clean:
	rm -f app.hex app.elf $(OBJECTS)

app.elf: $(OBJECTS)
	$(COMPILE) -o app.elf $(OBJECTS)

app.hex: app.elf
	rm -f app.hex
	avr-objcopy -j .text -j .data -O ihex app.elf app.hex
	avr-size --format=avr --mcu=$(DEVICE) app.elf
