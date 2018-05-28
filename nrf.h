#ifndef NRF_H
#define NRF_H

#include "pin.h"
#include "spi.h"
#include "nrf_defs.h"

struct CS_lock {
    Pin ce_lock;
    CS_lock(Pin ce_lock):ce_lock(ce_lock) {
        ce_lock = 0;
    }
    ~CS_lock() {
        ce_lock = 1;
    }
};

class NRF {
    private:
    public:
        const Pin irq,
              ce,
              cs;
        uint8_t read_reg8(uint8_t address) {
            CS_lock cl(cs);
            spi_send(address & 0x1F);
            return spi_send(0x00);
        }
        void write_reg8(uint8_t address, uint8_t data) {
            CS_lock cl(cs);
            spi_send(0x20 | (address & 0x1F));
            spi_send(data);
        }
        void read_regN(uint8_t address, uint8_t *data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(address & 0x1F);
            for(int i=0; i<length; i++)
                data[i] = spi_send(0x00);
        }
        void write_regN(uint8_t address, const uint8_t* data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(0x20 | (address & 0x1F));
            for(int i=0; i<length; i++)
                spi_send(data[i]);
        }
        uint8_t read_rx_pl_width() {
            CS_lock cl(cs);
            spi_send(0x60);
            return spi_send(0x00);
        }
        int read_rx_payload(uint8_t* data) {
            int length = read_rx_pl_width();
            CS_lock cl(cs);
            spi_send(0x61); // R_RX_PAYLOAD
            for(int i=0; i<length; i++)
                data[i] = spi_send(0x00);
            return length;
        }
        void write_tx_payload(const uint8_t* data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(0xA0);
            for(int i=0; i<length; i++)
                spi_send(data[i]);
        }
        void flush_tx() {
            CS_lock cl(cs);
            spi_send(0xE1);
        }
        void flush_rx() {
            CS_lock cl(cs);
            spi_send(0xE2);
        }
        void write_ack_payload(uint8_t pipe, const uint8_t* data, uint8_t length) {
            CS_lock cl(cs);
            spi_send(0xA8 | pipe);
            for(int i=0; i<length; i++)
                spi_send(data[i]);
        }

    //public:
        enum Mode {
            _RX,
            _TX
        };
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
        }
        void power_up() {
            uint8_t conf = read_reg8(ADDR_CONFIG);
            conf |= _BV(1);
            write_reg8(ADDR_CONFIG, conf);
            _delay_us(1500);
        }
        void power_down() {
            uint8_t conf = read_reg8(ADDR_CONFIG);
            conf &= ~_BV(1);
            write_reg8(ADDR_CONFIG, conf);
        }
        void set_bit(uint8_t reg_addr, uint8_t bit, uint8_t value) {
            uint8_t reg = read_reg8(reg_addr);
            if(value)
                reg |= _BV(bit);
            else
                reg &=~_BV(bit);
            write_reg8(reg_addr, reg);
        }

        void set_mode(Mode m) {
            set_bit(ADDR_CONFIG, 0, m == _RX);
        }

        void set_tx_addr(const uint8_t* data) {
            write_regN(ADDR_TX_ADDR, data, 5);
        }
        void set_freq(int freq) {
            write_reg8(ADDR_RF_CH, freq >= 2400 ? freq - 2400 : freq);
        }
        
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
                spi_send(0xE3);
            }
        }

        // assumes 5-byte address length
        //void setup_rx_pipe(int pipe, uint8_t *address){
        //    // enable shockburst auto-ack
        //    // enable RX address
        //    // set address width to 5
        //    // set auto-retransmit delay
        //}
        //void send(const uint8_t* data, uint8_t length) {
        //    write_tx_payload(data, length);
        //}
        //int available() { // return number of available bytes in last packet rx'd
        //}
        //int read(uint8_t* data) { // return received pipe number
        //}

};

#endif
