//////////////////////////////
// usart.h
//
// USART class for ATMega2560
// Copyright Aaron Schraner, 2018
// 
//
#ifndef USART_H
#define USART_H
#include <avr/io.h>
#include <avr/interrupt.h>
#include "circular_buffer.h"

typedef volatile uint8_t& reg_t;

// struct with references to all required USART configuration/data registers
struct USART_t {
    reg_t UDR,
          UCSRA,
          UCSRB,
          UCSRC;
    volatile uint16_t& UBRR;
};

template <int N> 
USART_t get_USART() {
    switch(N) {
        default:
        case 0: return USART_t { UDR0, UCSR0A, UCSR0B, UCSR0C, UBRR0 };
        case 1: return USART_t { UDR1, UCSR1A, UCSR1B, UCSR1C, UBRR1 };
        case 2: return USART_t { UDR2, UCSR2A, UCSR2B, UCSR2C, UBRR2 };
        case 3: return USART_t { UDR3, UCSR3A, UCSR3B, UCSR3C, UBRR3 };
    }
}

// pointers to any USART objects
void* USART_PTRS[4] = {
    0,
    0,
    0,
    0
};

// USART class
// usage: USART<N, BUFSIZE> my_usart(baudrate);
//  N is the USART number (e.g. USART<0> = USART0)
//  BUFSIZE is the size of RX buffer to use
//
//  the rx buffer is populated by the USART receive interrupt and cleared by the read() method
template <int N, int BUFSIZE=128>
class USART {
    private:
        USART_t usart; // references to config/data registers
        //CircularBuffer<uint8_t, BUFSIZE> tx_buffer;
        CircularBuffer<uint8_t, BUFSIZE> rx_buffer;

    public:
        USART(long baud = 9600, int stopbits = 1):
            usart(get_USART<N>())
        {
            select_baudrate(baud);
            usart.UCSRB = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
            usart.UCSRC = _BV(UCSZ00) | _BV(UCSZ01);
            if(stopbits == 2)
                usart.UCSRC |= _BV(USBS0);
            USART_PTRS[N] = this;
        }

        void rx_interrupt() {
            while( !(usart.UCSRA & _BV(RXC0)));
            rx_buffer.push(usart.UDR);
        }
        void select_baudrate(long baud) {
            // calculate baud
            usart.UBRR = F_CPU / 16 / baud - 1;
        }

        void send(uint8_t value) {
            while(!(usart.UCSRA & _BV(UDRE0)));
            usart.UDR = value;
        }

        void print(const char* value) {
            for(int i=0; value[i]; i++)
                send(value[i]);
        }

        // TODO: fix bug when printing the number 0
        void print(int number) {
            int size = number ? 0 : 1;
            int ncopy = number;
            char output[10];
            for(;ncopy;) {
                ncopy /= 10;
                size++;
            }
            ncopy = number;
            for(int i=0;ncopy;i++) {
                output[size - 1 - i] = ncopy % 10 + '0';
                ncopy /= 10;
            }
            for(int i=0; i<size; i++)
                send(output[i]);
        }
        bool available() {
            return rx_buffer.length() > 0;
        }
        uint8_t read() {
            return rx_buffer.pop();
        }
        uint8_t peek() {
            return rx_buffer.peek();
        }

};

template<int N>
void usart_rx_interrupt() {
    if(USART_PTRS[N])
        (reinterpret_cast<USART<N>*>(USART_PTRS[N])) -> rx_interrupt();
}

// USART interrupts push UDR onto rx_buffer
ISR(USART0_RX_vect) {
    usart_rx_interrupt<0>();
}

ISR(USART1_RX_vect) {
    usart_rx_interrupt<1>();
}

ISR(USART2_RX_vect) {
    usart_rx_interrupt<2>();
}

ISR(USART3_RX_vect) {
    usart_rx_interrupt<3>();
}

#endif
