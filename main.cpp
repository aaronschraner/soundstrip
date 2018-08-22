#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "pin.h"
#include "circular_buffer.h"
#include "timer.h"
#include "led_strip.h"
#include "usart.h"
#include "fix_fft.h"
#include "volume.h"
#include "nrf.h"


const int strip_length = 58; // number of LEDs on strip
const int fft_length = 128;  // number of samples for FFT

// USART for debugging (accessible over USB on arduino mega)
USART<0> usart(38400);

// sample buffer for FFT
// populated with ADC samples by timer interrupt
CircularBuffer<uint8_t, fft_length> circular_buffer;

// array of color objects representing LED strip
Color strip[strip_length];

// pins for rotary encoder 
Pin enc_gnd(PORTA, 5), // arduino pin 27
    enc_p1 (PORTA, 7), // arduino pin 29
    enc_p2 (PORTA, 3); // arduino pin 25
    
// class to control speaker volume by emulating a rotary encoder
VolumeControl volume(enc_p1, enc_p2, enc_gnd);

// Pins: (on arduino Mega)
//   IRQ | MOSI |   CS |  GND
//    CE | MISO |  SCK |  GND
// 
//   PL0 |  PB2 |  PB0 |  GND
//   PL1 |  PB3 |  PB1 |  GND
//
// Pins on NRF:
//    _________________________
//   ||     _____              |
//   ||    (16000) IRQ o o MISO|
//   |=== |  [ ]  MOSI o o SCK |
//   |=== |        CSN o o CE  |
//   |=== |        Vcc o o GND |
//   |____|____________________|
//
Pin nrf_irq(PORTL, 0, INPUT),
    nrf_ce(PORTL, 1, OUTPUT), // arduino mega pin 48
    nrf_cs(PORTB, 0, OUTPUT); // pin 53

// nRF24L01+ radio object
NRF nrf(nrf_irq, nrf_ce, nrf_cs);

// pin 13 on Arduino MEGA (has an LED on it)
Pin LED_pin(PORTB, 7, OUTPUT);

// clock and data pins for LED strip
Pin led_clk(PORTA, 0, OUTPUT);
Pin led_data(PORTA, 1, OUTPUT);

LEDStrip led_strip(led_clk, led_data, strip_length);

// real and imaginary buffers for FFT
char fft_buffer[fft_length];
char fft_ibuffer[fft_length];

// used by weighted-moving-average calculation
uint16_t strip_buffer[58];


// timer1 is used for sample clock, initiates conversion 
// and pushes last conversion result into circular_buffer.
// fft is run in main loop asynchronously
void adc_start_conversion();
void adc_init();

ISR(TIMER1_OVF_vect) {
    circular_buffer.push(ADC >> 2);
    adc_start_conversion();
}

ISR(ADC_vect) {
}

// ADC sample rate and downsample ratio
// samples are taken at a frequency of (samplerate / downsample) hertz
const int downsample = 16;
#define samplerate 44100

int abs(int value) {
    return value > 0 ? value : -value;
}

const uint8_t remote_address[6] = "2Node"; // remote address
const uint8_t station_address[6] = "1Node"; // receiver address
int main() {
    // initialize nRF module in TX mode
    nrf.init();
    nrf.setup_rx_pipe(1, station_address, 1); 
    nrf.start_listening();
    // initialize ADC and sample timer
    adc_init();
    sample_timer_init(samplerate / downsample);



    sei();
    const int alpha = 128;   // for WMA
    const int threshold = 8; // for LED strip

    while(1) {
        _delay_ms(20);

        // load circular buffer samples into FFT buffer
        for(int i=0; i<circular_buffer.length(); i++) {
            fft_buffer[i] = circular_buffer[i];
            fft_ibuffer[i] = 0;
        }
        
        // perform forward in-place FFT with m=2^7 (128) bins
        fix_fft(fft_buffer, fft_ibuffer, 7, 0);

        for(int i=0; i<strip_length; i++)
        {
            // approximate magnitude as abs(real)/2 + abs(imag)/2
            // multiply by 4 and calculate weighted moving average
            strip_buffer[i] = (strip_buffer[i] * (256 - alpha) + 
                    (abs(fft_buffer[i])/2 + abs(fft_ibuffer[i])/2) * 4 * alpha) / 256;

            // apply threshold
            const uint8_t intensity = strip_buffer[i] > threshold ? strip_buffer[i] - threshold / 2: 0;

            // convert sound intensity into color (TODO: make this mimic black-body radiation)
            strip[i] = Color(intensity, 
                    intensity > threshold * 4 ? (intensity - threshold * 4) / 4 : 0, 
                    0);
        }
        // update LED strip
        led_strip.draw(strip, 4);

        if(usart.available() || nrf.available()){
            uint8_t packet[32];
            if(usart.available())
                packet[0] = usart.read();
            else if(nrf.available())
            {
                nrf.stop_listening();
                nrf.read(packet);
                nrf.start_listening();
                //nrf.flush_rx();
                strip[0] = Color(0,0,64);
            }
                

            switch(packet[0]) {
                case '+': volume.up(); volume.up();volume.up(); strip[strip_length - 1] = Color(32); break;
                case '-': volume.down(); volume.down(); volume.down(); strip[0] = Color(32); break;
                case '?': 
                          strip[0] = volume.movable() ? Color(1, 255, 0) : Color(255, 0, 0); 
                          strip[1] = enc_p1 ? Color(64) : Color(0);
                          strip[2] = enc_p2 ? Color(64) : Color(0);
                          break;
                default: 
                          for(int i=0; i<8; i++)
                              strip[i] = Color((0x80 >> i) & packet[0] ? 64 : 0);
                          break;
            }
            led_strip.draw(strip);
            _delay_ms(20);
        }
        LED_pin = nrf[CD_REG] & 0x01;

        //for(int i=0; i<0x10; i++) {
        //    uint8_t reg = nrf.read_reg8(i);
        //    usart.print(i);
        //    usart.send(':');
        //    for(uint8_t b=0; b<8; b++)
        //        usart.send(reg & (0x80 >> b) ? '1' : '0');
        //    usart.send('\r');
        //    usart.send('\n');
        //    _delay_ms(1);
        //}
        //_delay_ms(500);

        
    }
}

void adc_start_conversion() {
    ADCSRA |= _BV(ADSC);
}

void adc_init() {
    //ADMUX = _BV(REFS0) | 0x0F; // ADC channel 1+/0-, Vcc reference, 200x gain
    ADMUX = _BV(REFS0) | 0x0D; // ADC channel 1+/0-, Vcc reference, 10x gain
    ADCSRA = _BV(ADEN) | _BV(ADIE); // enable ADC, enable interrupt
    ADCSRB = 0;
    DIDR0 |= _BV(2) | _BV(3);  // disable digital input buffer for channel 0 and 1
}

