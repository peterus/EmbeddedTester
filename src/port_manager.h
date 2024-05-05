#ifndef PORT_MANAGER_H_
#define PORT_MANAGER_H_

#include <list>

#include <Arduino.h>
#include <PubSubClient.h>

#include "port.h"

class PortManager {
public:
  PortManager(PubSubClient &client);

  void setBasicTopic(const String &basicTopic);
  void getMessage(const String &topic, const String &payload);

  void addPort(Port *port);

  void reconnect();
  void loop();

private:
  PubSubClient     &_client;
  String            _basicTopic;
  std::list<Port *> _ports;
  unsigned long     target_time;

  String getPortTopic(Port const *const port) const;

  void subscribe(Port const *const port);
  void updateStatus(Port const *const port);
  void updateMeasurement(Port *const port);
};

#endif
