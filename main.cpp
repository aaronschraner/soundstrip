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


const int strip_length = 58;
const int fft_length = 128;

USART<0> usart(115200);
CircularBuffer<uint8_t, fft_length> circular_buffer;
Color strip[strip_length];
Pin enc_gnd(PORTB, 2),
    enc_p1 (PORTB, 0),
    enc_p2 (PORTL, 0);
VolumeControl volume(enc_p1, enc_p2, enc_gnd);

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


// timer is used for sample clock - fires ADC interrupt
// on ADC completion, sample is pushed into circularbuffer
// on buffer fill, FFT is run on buffer. 

void adc_start_conversion();
void run_fft();
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

int main() {
    // initialize ADC and sample timer
    adc_init();
    sample_timer_init(samplerate / downsample);

    sei();
    const int alpha = 128;
    const int threshold = 8;
    while(1) {
        _delay_ms(20);
        for(int i=0; i<circular_buffer.length(); i++) {
            fft_buffer[i] = circular_buffer[i];
            fft_ibuffer[i] = 0;
        }
        //cli();
        //circular_buffer.flush();
        //sei();
        // perform forward in-place FFT with m=2^7 (128) bins
        fix_fft(fft_buffer, fft_ibuffer, 7, 0);
        for(int i=0; i<strip_length; i++)
        {
            // weighted moving average
            strip_buffer[i] = 
                (strip_buffer[i] * (256 - alpha) + (abs(fft_buffer[i])/2 + abs(fft_ibuffer[i])/2) * 4 * alpha) / 256;
            const uint8_t intensity = strip_buffer[i] > threshold ? strip_buffer[i] - threshold / 2: 0;
            strip[i] = Color(intensity, intensity > threshold * 4 ? (intensity - threshold * 4) / 4 : 0, 0);
        }

        led_strip.draw(strip);
        if(usart.available()) {
            switch(usart.read()) {
                case '+': volume.up(); strip[strip_length - 1] = Color(32); break;
                case '-': volume.down(); strip[0] = Color(32); break;
                case '?': 
                          strip[0] = volume.movable() ? Color(1, 255, 0) : Color(255, 0, 0); 
                          strip[1] = enc_p1 ? Color(64) : Color(0);
                          strip[2] = enc_p2 ? Color(64) : Color(0);
                          break;
                default: break;
            }
            led_strip.draw(strip);
            _delay_ms(100);
        }
    }
}

void adc_start_conversion() {
    ADCSRA |= _BV(ADSC);
}

void adc_init() {
    ADMUX = _BV(REFS0) | 0x0D; // ADC channel 1+/0-, Vcc reference
    ADCSRA = _BV(ADEN) | _BV(ADIE); // enable ADC, enable interrupt
    ADCSRB = 0;
    DIDR0 |= _BV(2) | _BV(3);  // disable digital input buffer for channel 0 and 1
}

