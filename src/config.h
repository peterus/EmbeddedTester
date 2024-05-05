#ifndef CONFIG_H_
#define CONFIG_H_

#include <LittleFS.h>

#include <ESP-Config.h>

class MQTT : public ObjectElement {
public:
  StringElement Server;
  IntElement    Port;

  MQTT() : ObjectElement("MQTT"), Server("Server", "192.168.88.5"), Port("Port", 1883) {
    register_element(&Server);
    register_element(&Port);
  }
};

class Config : public ObjectElement {
public:
  StringElement BoardName;
  MQTT          Mqtt;

  Config() : ObjectElement("Config"), BoardName("BoardName", "board1") {
    register_element(&BoardName);
    register_element(&Mqtt);
  }
};

void loadConfig(Config &config, File &file);
void saveConfig(Config &config, File &file);

#endif
