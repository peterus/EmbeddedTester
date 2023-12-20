#include <Arduino.h>
#include <LittleFS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "device.h"
#include "network.h"
#include "pins.h"
#include "webserver.h"

Device device1(DEVICE1_POWER_PIN);
Device device2(DEVICE2_POWER_PIN);

NetworkManager networkManager;
Webserver      webserver(&networkManager, &device1, &device2);

void setup() {
  Serial.begin(115200);
  sleep(5);

  Serial.println("Init LittleFS...");
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  Serial.println("Setup Network Manager...");
  networkManager.setup();
  Serial.println("Wait for Ethernet Connection...");
  networkManager.waitForConnection();

  Serial.println("Setup Webserver...");
  webserver.setup();

  Serial.println("setup finished!");
}

void loop() {
  networkManager.loop();
}
