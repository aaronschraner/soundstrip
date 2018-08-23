#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

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

// rotary encoder ISR
int knob_timer = 0;
ISR(PCINT2_vect) {
    // MCU re-enters sleep after this times out
    knob_timer = 3000; // 3000 = approx. 2 seconds

}


const uint8_t remote_address[6] = "2Node"; // remote address
const uint8_t station_address[6] = "1Node"; // receiver address
int main() {
    pullup_unused_pins();
    //NRF(Pin irq, Pin ce, Pin cs): irq(irq), ce(ce), cs(cs) {
    Pin nrf_irq(PORTD, 2, INPUT),
        nrf_ce (PORTB, 0, OUTPUT),
        nrf_cs (PORTB, 2, OUTPUT);

    Pin enc_a(PORTD, 6),
        enc_b(PORTD, 7);

    Encoder encoder;

    NRF nrf(nrf_irq, nrf_ce, nrf_cs);
    global_nrf = &nrf;

    nrf.init();
    nrf.set_freq(76);
    nrf.set_tx_addr(station_address);
    nrf.setup_rx_pipe(1, remote_address, 1);
    nrf.start_listening();
    _delay_ms(100);

    // send a '?' command on boot (check if speaker-side encoder is in the right position)
    uint8_t packet[1] = {'?'};
    led = 1;
    global_nrf->stop_listening();
    global_nrf->send(packet, 1);
    global_nrf->start_listening();
    led = 0;
    _delay_ms(10);

    // enable pin change interrupt on encoder pins - wakes MCU from power down sleep
    PCICR |= _BV(PCIE2);
    PCIFR |= _BV(PCIE2);
    PCMSK2 |= _BV(PCINT22) | _BV(PCINT23);

    // encoder will change volume - change_volume() gets invoked from within encoder.update().
    encoder.bind_callback(change_volume);

    // enable interrupts
    sei();

    while(1) {
        // enter power-down state
        nrf.power_down();
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_bod_disable(); // disable brownout detector while asleep
        //sei();
        sleep_cpu();
        sleep_disable();
        //cli();

        // power up NRF module
        nrf.power_up();

        // poll encoder and send packets to base station when knob is turned
        while(knob_timer) {
            encoder.update(enc_a << 1 | enc_b);
            _delay_us(10);
            knob_timer--;
        }
    }
}


