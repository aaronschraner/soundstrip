#include "spi.h"
#include <avr/io.h>
#include "pin.h"
#include <util/delay.h>

//TODO: implement SPI functions

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

uint8_t spi_send(uint8_t data)
{
    //const Pin 
    //    SCK(PORTB,  1, OUTPUT),
    //    MOSI(PORTB, 2, OUTPUT),
    //    MISO(PORTB, 3, INPUT);
    //SCK = 0;
    //uint8_t result = 0;
    //for(uint8_t x=0x80; x; x>>=1) {
    //    SCK = 1;
    //    MOSI = x;
    //    if(MISO)
    //        result |= x;
    //    SCK = 0;
    //}

    //return result;

	SPDR = data;
	while(!(SPSR & _BV(SPIF)));
    //_delay_us(600);
	return SPDR;
}

