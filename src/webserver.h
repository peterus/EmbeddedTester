#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "device.h"
#include "network.h"

class Webserver {
public:
  Webserver(NetworkManager *const networkManager, Device *const device1, Device *const device2);

  void setup();

private:
  AsyncWebServer        _server;
  NetworkManager *const _networkManager;
  Device *const         _device1;
  Device *const         _device2;

  void setup_wifi();
  void setup_device();
};

#endif
