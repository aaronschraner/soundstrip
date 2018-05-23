#microcontroller information
MCU=atmega2560
AVRDUDEMCU=$(MCU)
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude

CPPFILES=main.cpp timer.cpp fix_fft.cpp
CC=avr-g++
HFILES=pin.h circular_buffer.h
CFLAGS=-g -Os -std=c++11 -Wall -Wno-reorder -mcall-prologues -mmcu=$(MCU) -DF_CPU=$(CPU_FREQ) 
#PROGRAMMER=usbtiny
 PROGRAMMER=wiring
PORT=/dev/ttyACM1
TARGET=emre

###fuse configuration###
#use 8MHz internal clock
#LFUSE=0xE2
#CPU_FREQ=8000000UL
#use 1MHz internal clock
#LFUSE=0x62
#CPU_FREQ=1000000UL
# use 16MHz external clock
CPU_FREQ=16000000UL
LFUSE=0xFF
HFUSE=0xD8
EFUSE=0xFD

build: $(CPPFILES) $(HFILES)
	$(CC) $(CFLAGS) $(CPPFILES) -o $(TARGET).out
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET).out $(TARGET).hex

upload: build
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -c $(PROGRAMMER) \
		-P $(PORT) -D -U flash:w:$(TARGET).hex:i

fuse:
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -c $(PROGRAMMER) \
		-D -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m
		#-P $(PORT) -D -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m


clean:
	rm -fv $(TARGET).out $(TARGET).hex

