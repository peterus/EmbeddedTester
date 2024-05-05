#include "port.h"

Port::Port(size_t id, uint8_t pin, INA226 *const ina) : _id(id), _pin(pin), _ina(ina), _isActive(false) {
  pinMode(pin, OUTPUT);
}

size_t Port::getId() const {
  return _id;
}

INA226 *const Port::getINA() {
  return _ina;
}

void Port::on() {
  digitalWrite(_pin, HIGH);
  _isActive = true;
}

void Port::off() {
  digitalWrite(_pin, LOW);
  _isActive = false;
}

bool Port::isActive() const {
  return _isActive;
}
