#ifndef SMART_AQUARIUM_H
#define SMART_AQUARIUM_H

#include <Arduino.h>
#include <ArduinoJson.h> 


struct Settings {
  bool bubbler_isOn;
  bool display_isOn;


  int r_LED;
  int g_LED;
  int b_LED;

  String serialize() {
    StaticJsonDocument<200> doc; 
    doc["bubbler"] = bubbler_isOn;
    doc["display"] = display_isOn;
    doc["r"] = r_LED;
    doc["g"] = g_LED;
    doc["b"] = b_LED;
    String output;
    serializeJson(doc, output);
    return output;
  }
};

struct Readings {
  float tds_level;
  float water_temp;
  int daysSinceFed;

  // Convert Struct -> JSON String (For sending TO Phone)
  String serialize() {
    StaticJsonDocument<200> doc;
    
    // We trim floats to 2 decimal places to save space
    doc["tds"]   = serialized(String(tds_level, 2)); 
    doc["temp"]  = serialized(String(water_temp, 2));
    doc["fed"]   = daysSinceFed;

    String output;
    serializeJson(doc, output);
    return output;
  }
};



class SmartAquarium {
  public:
    // Constructor
    SmartAquarium(int pumpPin, int lightPin);

    // Core Methods
    void begin();
    void update();

    // Public Data Access
    Settings settings;
    Readings readings;

    // Helpers
    String getSettingsJSON();
    String getReadingsJSON();

  private:
    // Hardware Pins
    int _pumpPin;
    int _lightPin;

    // Internal Logic
    void readSensors();
    void applyHardwareState();
};

#endif // SMART_AQUARIUM_H