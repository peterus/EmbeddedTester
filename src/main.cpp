#include <array>

#include <Arduino.h>
#include <ETH.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>

#include <INA226.h>
#include <MqttLogger.h>

#include "config.h"
#include "network_manager.h"
#include "pins.h"
#include "port_manager.h"

Config         config;
NetworkManager networkManager;

WiFiClient   espClient;
PubSubClient mqttClient(espClient);
MqttLogger   mqttLogger(mqttClient, "", MqttAndSerial);

constexpr unsigned int INA226_BASE_ADDRESS = 0x40;

constexpr size_t                NUM_DEVICES = 3;
std::array<INA226, NUM_DEVICES> INAs        = {INA226(Wire), INA226(Wire), INA226(Wire)};
std::array<Port, NUM_DEVICES>   Ports       = {Port(0, DEVICE0_POWER_PIN, &INAs[0]), Port(1, DEVICE1_POWER_PIN, &INAs[1]), Port(2, DEVICE2_POWER_PIN, &INAs[2])};

PortManager portManager(mqttClient);

String basic_topic;

void mqtt_callback(char *_topic, uint8_t *_payload, unsigned int _length) {
  String topic(_topic);
  String payload = String(_payload, _length);
  mqttLogger.print("got topic: ");
  mqttLogger.print(topic);
  mqttLogger.print(", payload: ");
  mqttLogger.println(payload);
  portManager.getMessage(topic, payload);
}

void mqtt_publish(const String &topic, const String &payload, bool retained = false) {
  mqttClient.publish((basic_topic + topic).c_str(), payload.c_str(), retained);
}

void mqtt_reconnect() {
  mqttLogger.println("Attempting MQTT connection...");
  if (!mqttClient.connect("ESP32Logger", (basic_topic + "status").c_str(), 0, true, "offline")) {
    mqttLogger.print("failed, rc=");
    mqttLogger.print(mqttClient.state());
    mqttLogger.println(" try again in 5 seconds");
    delay(5000);
    return;
  }
  mqttLogger.println("connected.");
  mqtt_publish("status", "online", true);
  mqtt_publish("ip", ETH.localIP().toString(), true);
  portManager.reconnect();
}

#define INA_RSHUNT      0.1f
#define INA_MAX_CURRENT 2.0f

void setup() {
  Serial.begin(115200);
  sleep(5);
  mqttLogger.println("Embedded Tester by OE5BPA (Peter Buchegger)");

  if (!LittleFS.begin(true)) {
    mqttLogger.println("LittleFS Mount Failed");
    mqttLogger.println("continue with default values...");
  } else {
    File file = LittleFS.open("/config.json", FILE_READ, true);
    if (file) {
      loadConfig(config, file);
      file.close();
    }
  }

  File file = LittleFS.open("/config.json", FILE_WRITE, true);
  if (file) {
    saveConfig(config, file);
    file.close();
  }

  mqttLogger.println("Setup Network Manager...");
  networkManager.setup();
  mqttLogger.println("Wait for Ethernet Connection...");
  networkManager.waitForConnection();

  mqttLogger.print("MQTT Server: ");
  mqttLogger.print(config.Mqtt.Server.get());
  mqttLogger.print(", Port: ");
  mqttLogger.println(config.Mqtt.Port.get());

  IPAddress mqtt_server;
  mqtt_server.fromString(config.Mqtt.Server.get());
  mqttClient.setServer(mqtt_server, config.Mqtt.Port.get());
  mqttClient.setCallback(mqtt_callback);

  basic_topic = "embedded_tester/" + config.BoardName.get() + "/";
  mqttLogger.setTopic((basic_topic + "log").c_str());
  portManager.setBasicTopic(basic_topic);

  for (Port &port : Ports) {
    portManager.addPort(&port);
  }

  mqtt_reconnect();

  if (!Wire.begin(EXT_I2C_SDA, EXT_I2C_SLC)) {
    mqttLogger.println("issue with I2C!");
  } else {
    mqttLogger.println("I2C is working...");
    for (size_t i = 0; i < NUM_DEVICES; i++) {
      mqttLogger.print("INA226 on channel ");
      mqttLogger.print(i);
      mqttLogger.println(": start configure and calibration...");
      if (!INAs[i].begin(INA226_BASE_ADDRESS + i)) {
        mqttLogger.print("issue with INA on channel ");
        mqttLogger.print(i);
        mqttLogger.println("!");
        break;
      }
      if (!INAs[i].configure(INA226_AVERAGES_1, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT)) {
        mqttLogger.print("INA226 on channel ");
        mqttLogger.print(i);
        mqttLogger.println(": issue on configure!");
        break;
      }
      if (!INAs[i].calibrate(INA_RSHUNT, INA_MAX_CURRENT)) {
        mqttLogger.print("INA226 on channel ");
        mqttLogger.print(i);
        mqttLogger.println(": issue on calibrate!");
        break;
      }
      mqttLogger.print("INA226 on channel ");
      mqttLogger.print(i);
      mqttLogger.println(": configured and calibrated!");
    }
  }

  mqttLogger.println("setup finished!");
}

void loop() {
  if (!mqttClient.connected()) {
    mqtt_reconnect();
  }
  networkManager.loop();
  mqttClient.loop();
  portManager.loop();
}
