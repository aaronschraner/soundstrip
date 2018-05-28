#include "pin.h"
#ifndef LED_STRIP_H
#define LED_STRIP_H

struct Color {
  uint8_t r, g, b;

  Color(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) {}
  Color(uint8_t grey = 0): r(grey), g(grey), b(grey) {}
  Color(const Color& c): Color(c.r, c.g, c.b) {}


  const Color& operator=(const Color& c) {
    r = c.r;
    g = c.g;
    b = c.b;
    return *this;
  }
  operator uint32_t () const {
    return get_bgr();
  }

  uint32_t get_bgr() const {
    return ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
  }
};

class LEDStrip {
  public:
    LEDStrip(const Pin& clk, const Pin& data, int len):
      clk(clk), data(data), len(len) {
      clk.mode(OUTPUT);
      data.mode(OUTPUT);
    }

    template <typename T>
    void draw(const T& array, int brightness = 4) {
      send_start_frame();
      for (auto& pixel : array) {
        send32(((0xE0UL | brightness) << 24) | pixel.get_bgr());
      }
      send_end_frame();
    }

    void fill(const Color& c, int brightness = 4) {
      send_start_frame();
      for (int i = 0; i < len; i++)
        send32(((0xE0UL | brightness) << 24) | c.get_bgr());
      send_end_frame();
    }

  private:
    Pin clk, data;
    int len;
    inline void send_start_frame() const {
      send32(0UL); // send 32 zeros for start frame
    }
    inline void send_end_frame() const {
      send32(~0UL); // send 32 ones for end frame
    }
    void send32(uint32_t value) const {
      for (uint32_t r = 1UL << 31; r; r >>= 1) {
        clk = 0;
        //delayMicroseconds(1);
        data = value & r;
        //delayMicroseconds(1);
        clk = 1;
        //delayMicroseconds(1);

      }
    }
};

#endif
