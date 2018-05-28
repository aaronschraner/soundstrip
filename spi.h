#ifndef SPI_H
#define SPI_H
#include <stdint.h>

void spi_init();
uint8_t spi_send(uint8_t data);
// read one byte via SPI
inline uint8_t spi_read() {
    return spi_send(0x00);
}

#endif
