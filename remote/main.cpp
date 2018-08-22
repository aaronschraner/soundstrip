#include <avr/io.h>
#include <util/delay.h>

#include "../pin.h"
#include "../nrf.h"
#include "encoder.h"

// used pins:
// PB0 - NRF CE
// PB1 - LED
// PB2 - NRF CS
// PB3 - MOSI
// PB4 - MISO
// PB5 - SCK
// PC6 - RESET
// PD2 - NRF IRQ
// PD6 - ENC A
// PD7 - ENC B

void pullup_unused_pins() {
    PORTB |= 0xC0;
    PORTC |= 0x3F;
    PORTD |= 0x3B;
}

NRF* global_nrf = 0;
Pin led(PORTB, 1, OUTPUT);
void change_volume(int8_t increment, uint8_t edge_id) {
    if(edge_id != 0)
        return;
    uint8_t packet[1];
    switch(increment) {
        case 1:  packet[0] = '+'; break;
        case -1: packet[0] = '-'; break;
        default: packet[0] = '?'; break;
    }
    if(global_nrf)
    {
        led = 1;
        global_nrf->stop_listening();
        global_nrf->send(packet, 1);
        global_nrf->start_listening();
        led = 0;
    }
}

const uint8_t remote_address[6] = "2Node"; // remote address
const uint8_t station_address[6] = "1Node"; // receiver address
int main() {
    //NRF(Pin irq, Pin ce, Pin cs): irq(irq), ce(ce), cs(cs) {
    Pin nrf_irq(PORTD, 2),
        nrf_ce (PORTB, 0),
        nrf_cs (PORTB, 2);
    Pin enc_a(PORTD, 6),
        enc_b(PORTD, 7);
    Encoder encoder;

    NRF nrf(nrf_irq, nrf_ce, nrf_cs);
    global_nrf = &nrf;

    nrf.init();
    nrf.set_tx_addr(station_address);
    nrf.setup_rx_pipe(1, remote_address, 1);
    nrf.start_listening();
    _delay_ms(100);
    uint8_t packet[1] = {'?'};
    led = 1;
    global_nrf->stop_listening();
    global_nrf->send(packet, 1);
    global_nrf->start_listening();
    led = 0;
    _delay_ms(10);
    //nrf.set_freq(2412);
    //nrf.config_retransmission(10, 3); // 10 retransmits, 1ms delay

    // uint8_t packet[32];
    // for(int i=0; i<32; i++)
    //     packet[i] = 'A' + i;
    // packet[0] = '?';
    encoder.bind_callback(change_volume);
    while(1) {
        for(int i=0; i<8; i++) {
            encoder.update(enc_a << 1 | enc_b);
            //nrf.stop_listening();
            //led = 1;
            //packet[0] = i < 4 ? '?' : '?';
            //nrf.send(packet, 1); 
            //nrf.start_listening();
            //_delay_ms(50);
            //led = 0;
            //_delay_ms(950);
            _delay_ms(1);
        }
    }
}


