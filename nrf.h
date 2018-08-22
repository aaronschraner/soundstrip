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
            int i;
            for(i=0; i<length; i++)
                spi_send(data[i]);
            //for(;i<32; i++) // pad with zeros
            //    spi_send(0);
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
        class Reg8 { 
            private:
                NRF* const owner;
                const uint8_t address;
            public:
                Reg8(NRF* const owner, uint8_t address):
                    owner(owner), address(address) {}
                uint8_t read() const { 
                    return owner->read_reg8(address);
                }
                void write(uint8_t value) {
                    owner->write_reg8(address, value);
                }
                operator uint8_t() const { return read(); }
                Reg8& operator=(const uint8_t& value) { write(value); return *this;}
                uint8_t operator~() { return ~read();}
                //uint8_t operator&(uint8_t rhs) const { return read() & rhs; }
                //uint8_t operator^(uint8_t rhs) const { return read() ^ rhs; }
                //uint8_t operator|(uint8_t rhs) const { return read() | rhs; }
                uint8_t operator&=(uint8_t rhs) { *this = *this & rhs; return *this;};
                uint8_t operator^=(uint8_t rhs) { *this = *this ^ rhs; return *this;};
                uint8_t operator|=(uint8_t rhs) { *this = *this | rhs; return *this;};

        };
        Reg8 reg(NRF_register r) {
            return Reg8(this, *reinterpret_cast<uint8_t*>(&r));
        }
        Reg8 operator[] (NRF_register r) {
            return reg(r);
        }
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
            ce.set(0);
            cs.set(1);
        }
        void init() {
            _delay_ms(5); 
            // 2250us retransmit delay, max 15 retransmissions
            reg(SETUP_RETR) = (0b0100 << ARD) | (0b1111 << ARC); 
            
            reg(RF_SETUP) = (0x3 << RF_PWR) | (0 << RF_DR); // maximum power, 1Mbps data rate
            
            reg(CONFIG) = _BV(MASK_RX_DR) | _BV(MASK_TX_DS) | 
                _BV(MASK_MAX_RT) | _BV(EN_CRC) | _BV(CRC0); // 2-byte CRC enabled, interrupts disabled

            reg(DYNPD) = 0; // disable dynamic-length payloads
            reg(STATUS) = _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT); 
            set_freq(76); 
            flush_rx();
            flush_tx();
        }
        void power_up() {
            // set PWR_UP bit to 1
            set_bit(CONFIG, PWR_UP, 1);
            _delay_us(1500);
        }
        void power_down() {
            // set PWR_UP bit to 0
            set_bit(CONFIG, PWR_UP, 0);
        }

        // set transmitter target address (always assume 5-byte address)
        void set_tx_addr(const uint8_t* data, int length = 32) {
            write_regN(RX_ADDR_P0, data, 5);
            write_regN(TX_ADDR, data, 5);
            reg(RX_PW_P0) = length; //32-byte payload
        }

        // set radio frequency to <freq> MHz.
        void set_freq(int freq) {
            reg(RF_CH) = freq >= 2400 ? freq - 2400 : freq;
        }
        
        // Test carrier output power
        // from appendix C of nRF24L01 datasheet
        void broadcast_carrier(int frequency = 2400) {
            power_up(); // step 1, 2
            set_bit(CONFIG, PRIM_RX, 0); // put in TX mode (step 3)
            reg(EN_AA) = 0; // disable auto-ack (step 4)
            reg(RF_SETUP) = reg(RF_SETUP) | (0x03 << RF_PWR); // set output power to max (step 5)
            set_bit(RF_SETUP, PLL_LOCK, 1); // force PLL lock (step 6)
            uint8_t tx_addr[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            set_tx_addr(tx_addr); // set TX address to all 1's (step 7)
            uint8_t payload[32];
            for(int i=0; i<32; i++)
                payload[i] = 0xFF;
            write_tx_payload(payload, 32); // set payload to all 1's (step 8)
            set_bit(CONFIG, EN_CRC, 0); // disable CRC, step 9
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
            // from appendix A of nRF datasheet (Enhanced ShockBurst Receive Payload)
            // step 1
            // set_bit(EN_AA, pipe, 1); // enable auto-ack for pipe
            // set_bit(CONFIG, PRIM_RX, 1); // set RX mode
            if(pipe < 2)
                write_regN(RX_ADDR_P0 + pipe, address, 5);
            else
                write_reg8(RX_ADDR_P0 + pipe, address[4]);

            write_reg8(RX_PW_P0 + pipe, pl_length); // set payload width

            set_bit(EN_RXADDR, pipe, 1); // enable pipe
            // step 2
            // ce = 1; // activate RX mode
            // monitoring will begin after 130us (step 3)

        }
        bool available() { // return number of available bytes in last packet rx'd
            return reg(STATUS) & _BV(6); // read RX_DR bit in status register
            // appendix A (receive) step 4
        }

        // return received packet length, put pipe in <pipe> if present
        uint8_t read(uint8_t* data, uint8_t* pipe = 0) { 
            if(pipe) {
                uint8_t status = reg(STATUS);
                *pipe = (status & 0xE) >> 1; // fetch pipe number from RX_P_NO in status reg
            }
            bool ce_state = ce;
            ce = 0; // temporarily disable CE while data is read in
            int length = read_rx_payload(data);
            set_bit(STATUS, RX_DR, 1); // clear RX data ready interrupt
            ce = ce_state; // return CE to previous state
            return length;
        }

        // from appendix A of nRF datasheet
        // this method assumes that pipe 0 has already been configured with correct RX address
        void send(const uint8_t* data, uint8_t length) {
            set_bit(CONFIG, PRIM_RX, 0); // step 1
            _delay_us(150);
            write_tx_payload(data, length); // set TX payload data
            
            // begin transmitting 
            ce = 1;
            _delay_us(15); // at least 10us is required to guarantee successful transmission
            ce = 0;

            while(!(reg(STATUS) & (_BV(MAX_RT) | _BV(TX_DS))));
            power_down();
            flush_tx();
        }
        void config_retransmission(uint8_t retransmits, uint8_t delay) {
            reg(SETUP_RETR) = (delay << ARD) | (retransmits << ARC);
        }
        void start_listening() {
            //enable PRIM_RX and PWR_UP in config register
            reg(CONFIG) |= _BV(PWR_UP) | _BV(PRIM_RX);

            // clear interrupt requests
            reg(STATUS) = _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT);

            flush_rx();
            flush_tx();
            
            ce.set(HIGH);

            _delay_us(130);
        }

        void stop_listening() {
            ce.set(LOW);
            //flush_rx();
            //flush_tx();
        }

};

#endif
