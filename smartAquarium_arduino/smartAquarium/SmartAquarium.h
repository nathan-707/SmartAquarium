#ifndef SMART_AQUARIUM_H
#define SMART_AQUARIUM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>



enum class LightCycle : int {
  standard = 0,
  noSchedule = 1,
};

struct RGB {
  int red;
  int green;
  int blue;
  bool update;
};

struct Settings {

  // set defaults here. TODO: restore them from nvs, and save them when changed.
  bool bubbler_isOn;
  bool lamp_isOn = true;
  float temp_Warning_thres = 3;
  float targetTemp = 71;
  float tds_Warning_thres = 500;
  int daysFed_Warning_thres = 1;
  int r_LED = 0;
  int g_LED = 255;
  int b_LED = 0;
  float brightness = 1.0;
  LightCycle lightCycle = LightCycle::standard;
  int onTimeHr = 7;
  int onTimeMin = 0;
  int offTimeHr = 24;
  int offTimeMin = 0;

  String serialize() {
    StaticJsonDocument<400> doc;
    doc["bubbler"] = bubbler_isOn;
    doc["lamp"] = lamp_isOn;
    doc["temp_Warning_thres"] = temp_Warning_thres;
    doc["targetTemp"] = targetTemp;
    doc["tds_Warning_thres"] = tds_Warning_thres;
    doc["daysFed_Warning_thres"] = daysFed_Warning_thres;
    doc["r"] = r_LED;
    doc["g"] = g_LED;
    doc["b"] = b_LED;
    doc["brightness"] = brightness;
    doc["lightCycle"] = (int)lightCycle;
    doc["onTimeHr"] = onTimeHr;
    doc["onTimeMin"] = onTimeMin;
    doc["offTimeHr"] = offTimeHr;
    doc["offTimeMin"] = offTimeMin;
    String output;
    serializeJson(doc, output);
    return output;
  }
};

struct Readings {
  float tds_level;
  float water_temp;
  int daysSinceFed;
  bool waterLevel_isFull;
  bool lights_isOn;
  float turbidity;
  float pH;

  String serialize(bool tds_isOk, bool temp_isOk, bool daysFed_isOk, bool waterLevel_isOk) {
    StaticJsonDocument<400> doc;
    doc["tds"] = serialized(String(tds_level, 2));
    doc["temp"] = serialized(String(water_temp, 2));
    doc["fed"] = daysSinceFed;
    doc["tds_isOk"] = tds_isOk;
    doc["temp_isOk"] = temp_isOk;
    doc["daysFed_isOk"] = daysFed_isOk;
    doc["waterLevel_isOk"] = waterLevel_isOk;
    doc["lights_isOn"] = lights_isOn;
    doc["turbidity"] = turbidity;
    doc["pH"] = pH;
    String output;
    serializeJson(doc, output);
    return output;
  }
};



class SmartAquarium {
public:


  RGB standardLightCycle(int currentHour, int currentMin, int userOnHour, int userOnMin, int userOffHour, int userOffMin, bool testMode);
    // Constructor
    SmartAquarium(int pumpPin, int lightPin);

  // Core Methods
  void begin();
  void update();

  // Public Data Access
  Settings settings;
  Readings readings;
  bool sendReadingUpdateToApp;

private:
  // Hardware Pins
  int _pumpPin;
  int _lightPin;

  // Internal Logic
  void readSensors();
  void applyHardwareState();
};

#endif  // SMART_AQUARIUM_H