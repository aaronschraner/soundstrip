#ifndef NRF_H
#define NRF_H

//////////////////////////////
// nrf.h
//
// class for nRF24L01(+) low-power radio modules
// Copyright Aaron Schraner, 2018
// 

#include "pin.h"
#include "spi.h"
#include "nrf_defs.h" // configuration register address definitions

// helper struct for SPI devices
// pulls ce_lock low when constructed
// returns to 1 when destroyed
// used to guarantee that chip select line is returned to 1 when exiting functions
struct CS_lock {
    Pin ce_lock; // the CS pin
    CS_lock(Pin ce_lock):ce_lock(ce_lock) {
        ce_lock = 0; // pull CS low
    }
    ~CS_lock() {
        ce_lock = 1; // drive CS high
    }
};

// class for NRF radio
// TODO: fix private/public methods
class NRF {
    private:

        const Pin irq,
              ce,
              cs;

        // read an 8-bit configuration register at given address
        uint8_t read_reg8(uint8_t address) {
            CS_lock cl(cs);
            spi_send(address & 0x1F);
            return spi_read();
        }

        // write an 8-bit register value
        void write_reg8(uint8_t address, uint8_t data) {
            CS_lock cl(cs);
            spi_send(0x20 | (address & 0x1F));
            spi_send(data);
        }

        // read an N-bit configuration register
        void read_regN(uint8_t address, uint8_t *data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(address & 0x1F);
            for(int i=0; i<length; i++)
                data[i] = spi_read();
        }

        // write an N-bit configuration register
        void write_regN(uint8_t address, const uint8_t* data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(0x20 | (address & 0x1F));
            for(int i=0; i<length; i++)
                spi_send(data[i]);
        }

        // read the last received payload width
        uint8_t read_rx_pl_width() {
            CS_lock cl(cs);
            spi_send(0x60); // R_RX_PL_WID
            return spi_read();
        }

        // read the RX payload into a buffer
        // buffer must be at least 32 bytes to guarantee safety
        // returns length of payload in bytes
        int read_rx_payload(uint8_t* data) {
            int length = read_rx_pl_width(); //get payload length
            CS_lock cl(cs);
            spi_send(0x61); // R_RX_PAYLOAD

            // put the payload into <data>
            for(int i=0; i<length; i++)
                data[i] = spi_read();

            return length;
        }

        // write a payload to the TX payload register
        void write_tx_payload(const uint8_t* data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(0xA0);
            for(int i=0; i<length; i++)
                spi_send(data[i]);
        }

        // flush TX FIFO
        void flush_tx() {
            CS_lock cl(cs);
            spi_send(0xE1);
        }
        
        // flush RX FIFO
        void flush_rx() {
            CS_lock cl(cs);
            spi_send(0xE2);
        }

        // write a payload to be sent back to the transmitter along with the next ACK
        // (for a given pipe)
        void write_ack_payload(uint8_t pipe, const uint8_t* data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(0xA8 | pipe);
            for(int i=0; i<length; i++)
                spi_send(data[i]);
        }
        
        // set a given bit in 
        void set_bit(uint8_t reg_addr, uint8_t bit, uint8_t value) {
            uint8_t reg = read_reg8(reg_addr);
            if(value)
                reg |= _BV(bit);
            else
                reg &=~_BV(bit);
            write_reg8(reg_addr, reg);
        }

    public:
        enum Mode {
            _RX,
            _TX
        };

        // Constructor - initializes pins as output and initializes SPI
        NRF(Pin irq, Pin ce, Pin cs): irq(irq), ce(ce), cs(cs) {
            irq.mode(INPUT);
            ce.mode(OUTPUT);
            cs.mode(OUTPUT);
            spi_init();
        }
        void init(Mode m) {
            // set PWR_UP = 1 in config register
            write_reg8(ADDR_CONFIG, 0x7A | (m == _RX ? 1 : 0));
            // wait 1.5ms
            _delay_us(1500);
            set_bit(ADDR_CONFIG, 2, 1); // 2 byte CRC
            set_bit(ADDR_RF_SETUP, 3, 0); //1Mbps data rate
        }
        void power_up() {
            // set PWR_UP bit to 1
            set_bit(ADDR_CONFIG, 1, 1);
            _delay_us(1500);
        }
        void power_down() {
            // set PWR_UP bit to 0
            set_bit(ADDR_CONFIG, 1, 0);
        }
        void set_mode(Mode m) {
            set_bit(ADDR_CONFIG, 0, m == _RX);
        }

        // set transmitter target address (always assume 5-byte address)
        void set_tx_addr(const uint8_t* data) {
            write_regN(ADDR_TX_ADDR, data, 5);
        }

        // set radio frequency to <freq> MHz.
        void set_freq(int freq) {
            write_reg8(ADDR_RF_CH, freq >= 2400 ? freq - 2400 : freq);
        }
        
        // Test carrier output power
        // from appendix C of nRF24L01 datasheet
        void broadcast_carrier(int frequency = 2400) {
            power_up(); // step 1, 2
            set_mode(_TX); // step 3
            write_reg8(ADDR_EN_AA, 0x00); // step 4
            // leave output power at default (step 5)
            set_bit(ADDR_RF_SETUP, 4, 1); // step 6
            uint8_t tx_addr[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            set_tx_addr(tx_addr); // step 7
            uint8_t payload[32];
            for(int i=0; i<32; i++)
                payload[i] = 0xFF;
            write_tx_payload(payload, 32); // step 8
            set_bit(ADDR_CONFIG, 3, 0); // disable CRC, step 9
            set_freq(frequency); // step 10
            ce = 1;
            _delay_us(11);
            ce = 0; // pulse CE to send packet, step 11
            _delay_ms(1); // step 12
            ce = 1; // step 13
            {
                CS_lock cl(cs);
                spi_send(0xE3); //packet retransmit command, packets will repeat continuously until CE goes low
            }
        }

        // assumes 5-byte address length, forces fixed payload length
        void setup_rx_pipe(int pipe, const uint8_t *address, int pl_length){
            // from appendix A of nRF datasheet (Enhanced ShockBurst Receive Payload
            // step 1
            set_bit(ADDR_CONFIG, PRIM_RX, 1); // set RX mode
            set_bit(ADDR_EN_RXADDR, pipe, 1); // enable pipe
            set_bit(ADDR_EN_AA, pipe, 1); // enable auto-ack for pipe
            write_reg8(ADDR_RX_PW_P0 + pipe, pl_length); // set payload width
            if(pipe < 2)
                write_regN(ADDR_RX_ADDR_P0 + pipe, address, 5);
            else
                write_reg8(ADDR_RX_ADDR_P0 + pipe, address[4]);

            // step 2
            ce = 1; // activate RX mode
            // monitoring will begin after 130us (step 3)

        }
        bool available() { // return number of available bytes in last packet rx'd
            return read_reg8(ADDR_STATUS) & _BV(6); // read RX_DR bit in status register
            // appendix A (receive) step 4
        }

        // return received packet length, put pipe in <pipe> if present
        uint8_t read(uint8_t* data, uint8_t* pipe = 0) { 
            if(pipe) {
                uint8_t status = read_reg8(ADDR_STATUS);
                *pipe = (status & 0xE) >> 1; // fetch pipe number from RX_P_NO in status reg
            }
            bool ce_state = ce;
            ce = 0; // temporarily disable CE while data is read in
            int length = read_rx_payload(data);
            set_bit(ADDR_STATUS, 6, 1);
            ce = ce_state; // return CE to previous state
            return length;
        }

        // from appendix A of nRF datasheet
        // this method assumes that pipe 0 has already been configured with correct RX address
        void send(const uint8_t* data, uint8_t length, const uint8_t* address) {
            ce = 0;
            set_bit(ADDR_CONFIG, PRIM_RX, 0); // step 1
            set_tx_addr(address); // set TX address
            write_tx_payload(data, length); // set TX payload data
            write_regN(ADDR_RX_ADDR_P0, address, 5);
            
            // begin transmitting 
            ce = 1;
            _delay_ms(30); // at least 10us is required to guarantee successful transmission
            ce = 0;
        }
        void config_retransmission(uint8_t retransmits, uint8_t delay) {
            write_reg8(ADDR_SETUP_RETR, (delay << 4) | retransmits);
        }



};

#endif
