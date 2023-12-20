#include "device.h"

Device::Device(uint8_t pin, bool inverted, bool active) : _inverted(inverted), _pin(pin) {
  pinMode(_pin, OUTPUT);
  if (active) {
    activate();
  } else {
    deactivate();
  }
}

void Device::activate() {
  if (_inverted) {
    digitalWrite(_pin, LOW);
  } else {
    digitalWrite(_pin, HIGH);
  }
}

void Device::deactivate() {
  if (_inverted) {
    digitalWrite(_pin, HIGH);
  } else {
    digitalWrite(_pin, LOW);
  }
}
