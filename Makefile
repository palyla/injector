# DEVICE     = atmega328p
DEVICE     = atmega2560
CLOCK      = 16000000
AVRDUDE = avrdude -C /etc/avrdude.conf -c arduino -P /dev/ttyUSB0 -b 57600 -D -p $(DEVICE)

LIBS_H = -Ilib \
		 -Ilib/DL_Hamming \
		 -Ilib/lcd/nokia5110 \
		 -Ilib/lcd/T6963C
LIB_OBJECTS = lib/DL_Hamming/DL_Hamming.o \
		 lib/DL_Hamming/DL_HammingCalculateParitySmall.o \
		 lib/lcd/nokia5110/nokia5110.o \
		 lib/lcd/T6963C/T6963C.o

OBJECTS = $(LIB_OBJECTS) io/lcd.o io/uart.o app.o
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -Llib -ffunction-sections -fdata-sections -Wl,--gc-sections,-u,vfprintf,-lprintf_flt -lm
# COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -Llib -Wl,-u,vfprintf,-lprintf_flt -lm


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
