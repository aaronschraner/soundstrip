
#include "timer.h"

#include <avr/io.h>

void sample_timer_init(int sample_rate) {
    // set sample period 
    
    TIMSK1 |= _BV(TOIE1);

    TCCR1A = 0; 

    int prescale;
    // prescale = F_CPU / (ICR1 * sample_rate * 2)
    // ICR1_max = 65535
    if(sample_rate > 125)
        prescale = 1;
    else if(sample_rate > 16)
        prescale = 8;
    else if(sample_rate > 3)
        prescale = 64;
    else 
        prescale = 256;

    ICR1 = F_CPU / (sample_rate * 2 * prescale); // set output frequency
    


    TCCR1B = _BV(WGM13); // phase and frequency correct PWM, clear at ICR1

    // set prescale
    switch(prescale) {
        case 1:    TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10); break;
        case 8:    TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10); break;
        case 64:   TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10); break;
        case 256:  TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10); break;
        case 1024: TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10); break;
        default:  break;
    }


}


