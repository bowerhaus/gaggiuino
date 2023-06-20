#ifndef LED_H
#define LED_H

#include <stdint.h> // for uint8_t
#include <NCP5623.h>

class LED {
  public:
    LED() noexcept;
    void begin();
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void setRed(uint8_t red);
    void setBlue(uint8_t blue);
    void setGreen(uint8_t green);
    void setDisco(uint32_t timer);

  private:
    NCP5623 tankLED;
    uint32_t timer;
};
#endif
