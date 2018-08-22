//////////////////////////////
// nrf_defs.h
//
// configuration register address definitions for nRF24L01 
// Copyright Aaron Schraner, 2018
// 

#ifndef NRF_DEFS_H
#define NRF_DEFS_H

enum NRF_register: uint8_t {
    CONFIG = 0x00,
    EN_AA = 0x01,
    EN_RXADDR = 0x02,
    SETUP_AW = 0x03,
    SETUP_RETR = 0x04,
    RF_CH = 0x05,
    RF_SETUP = 0x06,
    STATUS = 0x07,
    OBSERVE_TX = 0x08,
    CD_REG = 0x09,
    RX_ADDR_P0 = 0x0A,
    RX_ADDR_P1 = 0x0B,
    RX_ADDR_P2 = 0x0C,
    RX_ADDR_P3 = 0x0D,
    RX_ADDR_P4 = 0x0E,
    RX_ADDR_P5 = 0x0F,
    TX_ADDR = 0x10,
    RX_PW_P0 = 0x11,
    RX_PW_P1 = 0x12,
    RX_PW_P2 = 0x13,
    RX_PW_P3 = 0x14,
    RX_PW_P4 = 0x15,
    RX_PW_P5 = 0x16,
    FIFO_STATUS = 0x17,
    DYNPD = 0x1C,
    FEATURE = 0x1D
};

// CONFIG (0x00)
#define         MASK_RX_DR  6
#define         MASK_TX_DS  5
#define         MASK_MAX_RT 4
#define         EN_CRC      3
#define         CRC0        2
#define         PWR_UP      1
#define         PRIM_RX     0

// EN_AA (0x01)
#define         ENAA_P5     5
#define         ENAA_P4     4
#define         ENAA_P3     3
#define         ENAA_P2     2
#define         ENAA_P1     1
#define         ENAA_P0     0

// EN_RXADDR (0x02)
#define         ERX_P5      5
#define         ERX_P4      4
#define         ERX_P3      3
#define         ERX_P2      2
#define         ERX_P1      1
#define         ERX_P0      0

// SETUP_AW (0x03)
#define         AW          0 // bits 1 and 0

// SETUP_RETR
#define         ARD         4 //bits 7:4
#define         ARC         0 //bits 3:0

// RF_CH (0x05)
// RF_SETUP (0x06)
#define         PLL_LOCK    4
#define         RF_DR       3
#define         RF_PWR      1 //bits 2:1
#define         LNA_HCURR   0

// STATUS (0x07)
#define         RX_DR       6
#define         TX_DS       5
#define         MAX_RT      4
#define         RX_P_NO     1 //bits 3:1
#define         TX_FULL     0

// OBSERVE_TX (0x08)
#define         PLOS_CNT    4 //bits 7:4
#define         ARC_CNT     0 //bits 3:0

// CD (0x09)
#define         CD          0 // carrier detect

#endif
