#include <ArduinoJson.h>

#include "port_manager.h"

constexpr unsigned long PERIOD(1 * 1000L); // 1 sek

PortManager::PortManager(PubSubClient &client) : _client(client), target_time(0) {
}

void PortManager::setBasicTopic(const String &basicTopic) {
  _basicTopic = basicTopic;
}

void PortManager::getMessage(const String &topic, const String &payload) {
  for (auto port : _ports) {
    String portTopic = getPortTopic(port) + "activate";
    if (topic == portTopic) {
      if (payload == "on") {
        port->on();
      } else {
        port->off();
      }
      updateStatus(port);
    }
  }
}

void PortManager::addPort(Port *port) {
  _ports.push_back(port);
}

void PortManager::reconnect() {
  for (const auto port : _ports) {
    subscribe(port);
    updateStatus(port);
  }
}

void PortManager::loop() {
  if (millis() - target_time >= PERIOD) {
    target_time += PERIOD;
    for (const auto port : _ports) {
      if (port->isActive()) {
        updateMeasurement(port);
      }
    }
  }
}

String PortManager::getPortTopic(Port const *const port) const {
  String portTopic = _basicTopic;
  portTopic += "port";
  portTopic += port->getId();
  portTopic += "/";
  return portTopic;
}

void PortManager::subscribe(Port const *const port) {
  String portTopic = getPortTopic(port) + "activate";
  _client.subscribe(portTopic.c_str());
}

void PortManager::updateStatus(Port const *const port) {
  String status("off");
  if (port->isActive()) {
    status = "on";
  }
  _client.publish((getPortTopic(port) + "status").c_str(), status.c_str(), true);
}

void PortManager::updateMeasurement(Port *const port) {
  DynamicJsonDocument doc(256);
  doc["voltage"] = port->getINA()->readBusVoltage();
  doc["current"] = port->getINA()->readShuntCurrent();
  doc["power"]   = port->getINA()->readBusPower();
  String buffer;
  serializeJson(doc, buffer);

  _client.publish((getPortTopic(port) + "measurement").c_str(), buffer.c_str());
}
