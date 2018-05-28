#include "spi.h"
#include <avr/io.h>
//////////////////////////////
// spi.cpp
//
// driver for hardware SPI (only on ATMEGA2560 right now)
// Copyright Aaron Schraner, 2018
// 

#include "pin.h"
#include <util/delay.h>

void spi_init()
{
    const Pin 
        SCK(PORTB,  1, OUTPUT),
        MOSI(PORTB, 2, OUTPUT),
        MISO(PORTB, 3, INPUT),
        _SS(PORTB,  0, OUTPUT); // if _SS is not configured as an output, SPI does not work
    _SS = 1;

	//enable the SPI module
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPI2X);
}

// send <data>, MSB first
// return SPI input data
uint8_t spi_send(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & _BV(SPIF)));
	return SPDR;
}

