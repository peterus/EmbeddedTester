#ifndef NETWORK_H_
#define NETWORK_H_

#include <Arduino.h>

class NetworkManager {
public:
  NetworkManager();

  void setup();
  void loop();

  void activateWifiAP(const String &ssid);
  void deactivateWifiAP();

  String getWifiPassphrase();

  void waitForConnection();

private:
  String _passphrase;

  bool setupEthernet();
  void setupOTA();
};

#endif
