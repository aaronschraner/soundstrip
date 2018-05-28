//////////////////////////////
// volume.h
//
// class for emulating a rotary encoder
// this works by connecting in parallel with a rotary encoder
// when both contacts of the actual rotary encoder are open, this program 
// can mimic a rotary encoder by pulling two digital pins to ground in a sequence
//
// this design only works for encoders that use pull-up resistors on the encoder pins
// and takes advantage of the wired-NAND properties of this circuit
//
// Copyright Aaron Schraner, 2018
// 
//
#ifndef VOLUME_H
#define VOLUME_H

class VolumeControl {
    private:
        const Pin e1, e2, gnd; //encoder pins 1 and 2, ground pin for easy connection
        int state;
        void output() const {
            // call this after incrementing or decrementing the state
            // pins assert low when bits 1 (e1) or 0 (e2) are set high
            const static uint8_t states[4] = {0, 2, 3, 1};
            e1.mode(states[state] & 2 ? OUTPUT : INPUT);
            e2.mode(states[state] & 1 ? OUTPUT : INPUT);
            _delay_ms(2);
        }

    public:
        // set up pins, start with both open (encoder unaffected)
        VolumeControl(const Pin& e1, const Pin& e2, const Pin& gnd): 
            e1(e1), e2(e2), gnd(gnd), state(0) {
            gnd.set(0);
            e1.set(0);
            e2.set(0);
            gnd.mode(OUTPUT);
            output();
        }
        
        // volume up
        inline void up() {
            state = (state + 1) % 4;
            output();
        }

        // volume down
        inline void down() {
            state = (state + 3) % 4;
            output();
        }

        // returns true if physical rotary encoder is at a position
        // that allows proper functioning of the code
        // also, resets state to 0 (both pins unconnected)
        bool movable() {
            state = 0;
            output();
            _delay_ms(5);
            bool result = e1 && e2;
            return result;
        }
};



#endif
