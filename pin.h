#ifndef PIN_H
#define PIN_H

enum Direction {
    INPUT,
    OUTPUT
};

enum PinValue: bool {
    HIGH = true,
    LOW = false
};

struct Pin {

    volatile uint8_t & port ;
    uint8_t pin;

    Pin(volatile uint8_t& port, uint8_t pin, Direction d = INPUT):
        port(port), pin(pin)
    {
        mode(d);
        set(0);
    }
    void mode(Direction d) const {
        ddr_reg() = (d == OUTPUT) ? ddr_reg() | _BV(pin) : ddr_reg() & ~_BV(pin);
    }

    void set(bool value) const {
        port = value ? 
            port | _BV(pin) :
            port &~_BV(pin);
    }
    
    bool get() const {
        return pin_reg() & _BV(pin);
    }

    volatile uint8_t& port_reg() const {
        return port;
    }

    volatile uint8_t& ddr_reg() const {
        return *(&port + (&DDRB - &PORTB));
    }

    volatile uint8_t& pin_reg() const {
        return *(&port + (&PINB - &PORTB));
    }
    const Pin& operator=(bool value) const {
        set(value);
        return *this;
    }
    operator bool() const {
        return get();
    }


};

#endif
