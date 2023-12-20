#ifndef POWER_H_
#define POWER_H_

#include <Arduino.h>

class Device {
public:
  Device(uint8_t pin, bool inverted = false, bool active = false);

  void activate();
  void deactivate();

private:
  bool    _inverted;
  uint8_t _pin;
};

#endif
