#include <avr/io.h>
#include <util/delay.h>

#include "../pin.h"
#include "../nrf.h"


const uint8_t remote_address[6] = "2Node"; // remote address
const uint8_t station_address[6] = "1Node"; // receiver address
int main() {
    Pin led(PORTB, 1, OUTPUT);
    //NRF(Pin irq, Pin ce, Pin cs): irq(irq), ce(ce), cs(cs) {
    Pin nrf_irq(PORTD, 2),
        nrf_ce (PORTB, 0),
        nrf_cs (PORTB, 2);

    NRF nrf(nrf_irq, nrf_ce, nrf_cs);

    nrf.init();
    nrf.set_tx_addr(station_address);
    //nrf.set_freq(2412);
    //nrf.config_retransmission(10, 3); // 10 retransmits, 1ms delay
    //nrf.setup_rx_pipe(0, nrf_address, 1);

    uint8_t packet[32];
    for(int i=0; i<32; i++)
        packet[i] = 'A' + i;
    packet[0] = '?';
    while(1) {
        nrf.send(packet, 32);
        led = 1;
        _delay_ms(1000);
        nrf.power_down();
        led = 0;
        _delay_ms(1000);
    }
}


