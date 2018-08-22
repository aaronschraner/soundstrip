#ifndef ENCODER_BARGRAPH_H
#define ENCODER_BARGRAPH_H
#include <avr/io.h>

typedef void(*EncoderCallback)(int8_t delta, uint8_t edge_id);

class Encoder {
    private:
        uint8_t prev_state;
        EncoderCallback callback;

    public:
        Encoder(EncoderCallback callback = 0): callback(callback), prev_state(3) {}
        void update(uint8_t new_state); //runs callback if encoder state has changed
        void bind_callback(EncoderCallback cb);

};

#endif
