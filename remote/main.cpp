#include <avr/io.h>
#include <util/delay.h>

#include "../pin.h"
#include "../nrf.h"


const uint8_t nrf_address[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE}; // RX address
int main() {
    Pin led(PORTB, 1, OUTPUT);
    //NRF(Pin irq, Pin ce, Pin cs): irq(irq), ce(ce), cs(cs) {
    Pin nrf_irq(PORTD, 2),
        nrf_ce (PORTB, 0),
        nrf_cs (PORTB, 2);

    NRF nrf(nrf_irq, nrf_ce, nrf_cs);

    nrf.init(NRF::_TX);
    //nrf.set_freq(2412);
    //nrf.config_retransmission(10, 3); // 10 retransmits, 1ms delay
    //nrf.setup_rx_pipe(0, nrf_address, 1);

    uint8_t packet[32];
    for(int i=0; i<32; i++)
        packet[i] = 0;
    while(1) {
        packet[0] = '-';
        nrf.broadcast_carrier(2412);
        led = 1;
        _delay_ms(1000);
        nrf.power_down();
        led = 0;
        _delay_ms(1000);
    }
}


