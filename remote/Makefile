#microcontroller information
MCU=atmega328
AVRDUDEMCU=m328
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude

CPPFILES=main.cpp ../spi.cpp encoder.cpp
CC=avr-g++
HFILES=encoder.h ../nrf_defs.h ../nrf.h
CPU_FREQ=1000000UL
CFLAGS=-g -Os -std=c++11 -Wall -Wno-reorder -mcall-prologues -mmcu=$(MCU) -DF_CPU=$(CPU_FREQ)
PROGRAMMER=usbasp
TARGET=remote

###fuse configuration###
#use 8MHz internal clock
#LFUSE=0xE2
#use 1MHz internal clock
LFUSE=0x62
HFUSE=0xD9
EFUSE=0xFF

build: $(CPPFILES) $(HFILES)
	$(CC) $(CFLAGS) $(CPPFILES) -o $(TARGET).out
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET).out $(TARGET).hex

upload: build
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

fuse:
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -c $(PROGRAMMER) \
		-U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m


clean:
	rm -fv $(TARGET).out $(TARGET).hex

