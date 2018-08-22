#include "encoder.h"

void Encoder::update(uint8_t new_state) {
    // state transitions:
    // id 0     1     2     3
    // 11 -> 01 -> 00 -> 10 -> 11 (clockwise)
    //
    // id 3     2     1     0
    // 11 -> 10 -> 00 -> 01 -> 11 (c-clockwise)
    //
    //     11   
    // 3 /    \ 0
    //  10    01
    // 2 \    / 1
    //     00
    // convert grey code (index) to binary
    const uint8_t index = (prev_state << 2) | new_state;
    const int8_t deltas[16] = {
        //       00  01 -10 -11
        /*00xx*/  0,  1, -1,  0,
        /*01xx*/ -1,  0,  0,  1,
        /*10xx*/  1,  0,  0, -1,
        /*11xx*/  0, -1,  1,  0};

    const uint8_t edge_ids[16] = {
        //       00  01  10  11
        /*00xx*/  0,  1,  2,  0,
        /*01xx*/  1,  0,  0,  0,
        /*10xx*/  2,  0,  0,  3,
        /*11xx*/  0,  0,  3,  0};

    if(prev_state != new_state && (prev_state ^ new_state) != 0x03)
        if(callback)
            callback(deltas[index], edge_ids[index]);
    prev_state = new_state;
}

void Encoder::bind_callback(EncoderCallback cb) {
    callback = cb;
}

