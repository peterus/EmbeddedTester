#ifndef PORT_H_
#define PORT_H_

#include <Wire.h>

#include <INA226.h>

class Port;

class Port {
public:
  Port(size_t id, uint8_t pin, INA226 *const ina);

  size_t        getId() const;
  INA226 *const getINA();

  void on();
  void off();

  bool isActive() const;

private:
  const size_t  _id;
  const uint8_t _pin;
  INA226 *const _ina;
  bool          _isActive;
};

#endif
