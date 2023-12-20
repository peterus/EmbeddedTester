#include <LittleFS.h>

#include "network.h"
#include "webserver.h"

void index_func(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Hello World!");
}

Webserver::Webserver(NetworkManager *const networkManager, Device *const device1, Device *const device2) : _server(80), _networkManager(networkManager), _device1(device1), _device2(device2) {
}

void Webserver::setup() {
  /*_server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", String(), false, [](const String &var) {
      if (var == "HELLO_FROM_TEMPLATE")
        return F("Hello world!");
      return F("");
    });
  });*/
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello World!");
  });
  /*_server.on("/form.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/form.css");
  });
  _server.on("/menu.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/menu.css");
  });*/

  setup_wifi();
  setup_device();

  _server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  _server.begin();
}

void Webserver::setup_wifi() {
  _server.on("/api/wifi/activate", HTTP_GET, [&](AsyncWebServerRequest *request) {
    _networkManager->activateWifiAP("ESP32-testAP");
    Serial.print("ESP Password: ");
    Serial.println(_networkManager->getWifiPassphrase());
    char buffer[100];
    sprintf(buffer, "AP is now running....\nPassword: %s", _networkManager->getWifiPassphrase().c_str());
    request->send(200, "text/plain", buffer);
  });

  _server.on("/api/wifi/deactivate", HTTP_GET, [&](AsyncWebServerRequest *request) {
    _networkManager->deactivateWifiAP();
    request->send(200, "text/plain", "AP is now deactivated!");
  });
}

void Webserver::setup_device() {
  _server.on("/api/device1/activate", HTTP_GET, [&](AsyncWebServerRequest *request) {
    _device1->activate();
    request->send(200, "text/plain", "Device 1 is now active!");
  });

  _server.on("/api/device1/deactivate", HTTP_GET, [&](AsyncWebServerRequest *request) {
    _device1->deactivate();
    request->send(200, "text/plain", "Device 1 is now deactivated!");
  });

  _server.on("/api/device2/activate", HTTP_GET, [&](AsyncWebServerRequest *request) {
    _device2->activate();
    request->send(200, "text/plain", "Device 2 is now active!");
  });

  _server.on("/api/device2/deactivate", HTTP_GET, [&](AsyncWebServerRequest *request) {
    _device2->deactivate();
    request->send(200, "text/plain", "Device 2 is now deactivated!");
  });
}
