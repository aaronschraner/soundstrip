#ifndef VOLUME_H
#define VOLUME_H

class VolumeControl {
    private:
        Pin e1, e2, gnd;
        int state;
        void output() const {
            // call this after incrementing or decrementing the state
            // pins assert low when bit 1 (e1) or 0 (e2) are set high
            const static uint8_t states[4] = {0, 2, 3, 1};
            e1.mode(states[state] & 2 ? OUTPUT : INPUT);
            e2.mode(states[state] & 1 ? OUTPUT : INPUT);
            _delay_ms(2);
        }

    public:
        VolumeControl(Pin e1, Pin e2, Pin gnd): e1(e1), e2(e2), gnd(gnd), state(0) {
            gnd.set(0);
            e1.set(0);
            e2.set(0);
            gnd.mode(OUTPUT);
            output();
        }
        inline void up() {
            state = (state + 1) % 4;
            output();
            state = (state + 1) % 4;
            output();
        }

        inline void down() {
            state = (state + 3) % 4;
            output();
            state = (state + 3) % 4;
            output();
        }
        bool movable() {
            state = 0;
            output();
            _delay_ms(5);
            bool result = e1 && e2;
            return result;
        }
};



#endif
