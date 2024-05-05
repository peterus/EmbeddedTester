#include "config.h"

void loadConfig(Config &config, File &file) {
  DynamicJsonDocument doc(256);

  deserializeJson(doc, file);
  JsonObject obj = doc.as<JsonObject>();
  config.load(obj);
}

void saveConfig(Config &config, File &file) {
  DynamicJsonDocument doc(256);

  JsonObject obj = doc.to<JsonObject>();
  config.save(obj);

  serializeJson(doc, file);
}
