//////////////////////////////
// nrf_defs.h
//
// configuration register address definitions for nRF24L01 
// Copyright Aaron Schraner, 2018
// 

#ifndef NRF_DEFS_H
#define NRF_DEFS_H

enum NRF_addresses: uint8_t {
    ADDR_CONFIG = 0x00,
    ADDR_EN_AA = 0x01,
    ADDR_EN_RXADDR = 0x02,
    ADDR_SETUP_AW = 0x03,
    ADDR_SETUP_RETR = 0x04,
    ADDR_RF_CH = 0x05,
    ADDR_RF_SETUP = 0x06,
    ADDR_STATUS = 0x07,
    ADDR_OBSERVE_TX = 0x08,
    ADDR_CD = 0x09,
    ADDR_RX_ADDR_P0 = 0x0A,
    ADDR_RX_ADDR_P1 = 0x0B,
    ADDR_RX_ADDR_P2 = 0x0C,
    ADDR_RX_ADDR_P3 = 0x0D,
    ADDR_RX_ADDR_P4 = 0x0E,
    ADDR_RX_ADDR_P5 = 0x0F,
    ADDR_TX_ADDR = 0x10,
    ADDR_RX_PW_P0 = 0x11,
    ADDR_RX_PW_P1 = 0x12,
    ADDR_RX_PW_P2 = 0x13,
    ADDR_RX_PW_P3 = 0x14,
    ADDR_RX_PW_P4 = 0x15,
    ADDR_RX_PW_P5 = 0x16,
    ADDR_FIFO_STATUS = 0x17,
    ADDR_DYNPD = 0x1C,
    ADDR_FEATURE = 0x1D
};

// TODO: make table of all bit positions in conf regs

// CONFIG

#define PRIM_RX 0

#endif
