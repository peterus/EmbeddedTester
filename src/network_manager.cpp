#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ETH.h>
#include <SPI.h>
#include <WiFi.h>

#include <lwip/dns.h>
#include <lwip/lwip_napt.h>

#include <MqttLogger.h>

#include "network_manager.h"
#include "pins.h"
#include "random_string.h"

extern MqttLogger    mqttLogger;
static volatile bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case ARDUINO_EVENT_ETH_START:
    mqttLogger.println("ETH Started");
    ETH.setHostname("esp32-ethernet");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    mqttLogger.println("ETH Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    mqttLogger.print("ETH MAC: ");
    mqttLogger.print(ETH.macAddress());
    mqttLogger.print(", IPv4: ");
    mqttLogger.print(ETH.localIP());
    if (ETH.fullDuplex()) {
      mqttLogger.print(", FULL_DUPLEX");
    }
    mqttLogger.print(", ");
    mqttLogger.print(ETH.linkSpeed());
    mqttLogger.print("Mbps");
    mqttLogger.print(", ");
    mqttLogger.print("GatewayIP: ");
    mqttLogger.print(ETH.gatewayIP());
    mqttLogger.print(", ");
    mqttLogger.print("DNS IP: ");
    mqttLogger.println(ETH.dnsIP());
    eth_connected = true;
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    mqttLogger.println("ETH Disconnected");
    eth_connected = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    mqttLogger.println("ETH Stopped");
    eth_connected = false;
    break;
  default:
    break;
  }
}

NetworkManager::NetworkManager() {
}

void NetworkManager::setup() {
  setupEthernet();
  setupOTA();
}

void NetworkManager::loop() {
  ArduinoOTA.handle();
}

bool NetworkManager::setupEthernet() {
  WiFi.onEvent(WiFiEvent);

  SPI.begin(ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN);
  if (!ETH.begin(ETH_PHY_W5500, ETH_ADDR, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN, SPI)) {
    mqttLogger.println("ETH start Failed!");
    return false;
  }

  ip_napt_enable(WiFi.softAPIP(), 1);
  return true;
}

void NetworkManager::setupOTA() {
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        mqttLogger.println("Start updating " + type);
      })
      .onEnd([]() {
        mqttLogger.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        mqttLogger.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        mqttLogger.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          mqttLogger.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          mqttLogger.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          mqttLogger.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          mqttLogger.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          mqttLogger.println("End Failed");
      });

  ArduinoOTA.begin();
}

void NetworkManager::activateWifiAP(const String &ssid) {
  _passphrase = "";
  get_random_string(_passphrase, 10);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  mqttLogger.printf("Creating softAP with passphrase: '%s'\n", _passphrase.c_str());
  IPAddress softIP(192, 168, 4, 1);
  WiFi.softAP(ssid, _passphrase);
  WiFi.softAPConfig(softIP, softIP, IPAddress(255, 255, 255, 0));
  mqttLogger.print("IP address:\t");
  mqttLogger.println(WiFi.softAPIP());
}

void NetworkManager::deactivateWifiAP() {
  WiFi.softAPdisconnect(true);
}

String NetworkManager::getWifiPassphrase() {
  return _passphrase;
}

void NetworkManager::waitForConnection() {
  while (!eth_connected) {
    sleep(1);
  }
}
