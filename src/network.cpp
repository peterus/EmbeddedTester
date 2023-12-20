#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ETH.h>
#include <WiFi.h>

#include <lwip/dns.h>
#include <lwip/lwip_napt.h>

#include "network.h"
#include "pins.h"
#include "random_string.h"

static volatile bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case ARDUINO_EVENT_ETH_START:
    Serial.println("ETH Started");
    ETH.setHostname("esp32-ethernet");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex()) {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.print("Mbps");
    Serial.print(", ");
    Serial.print("GatewayIP:");
    Serial.println(ETH.gatewayIP());
    eth_connected = true;
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    eth_connected = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
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
    Serial.println("ETH start Failed!");
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
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
}

void NetworkManager::activateWifiAP(const String &ssid) {
  get_random_string(_passphrase, 10);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.printf("Creating softAP with passphrase: '%s'\n", _passphrase.c_str());
  IPAddress softIP(192, 168, 4, 1);
  WiFi.softAP(ssid, _passphrase);
  WiFi.softAPConfig(softIP, softIP, IPAddress(255, 255, 255, 0));
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
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
