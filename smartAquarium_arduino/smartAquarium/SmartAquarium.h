#ifndef SMART_AQUARIUM_H
#define SMART_AQUARIUM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <Preferences.h>  // Include Preferences Library

enum class WifiStatus : int {
  connecting = 0,
  connected = 1,
  wifiError = 2,
};

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
  String ssid = "";
  String password = "";
};

struct Readings {
  float tds_level;
  float water_temp;
  int daysSinceFed;
  bool waterLevel_isFull;
  bool lights_isOn;
  float turbidity;
  float pH;
  WifiStatus status = WifiStatus::connecting;
};

class SmartAquarium {
public:
  SmartAquarium(int pumpPin, int lightPin);
  String serializeSettings();
  String serializeReadings();
  RGB standardLightCycle(bool testMode = false, int testHr = 0, int testMin = 0);
  void begin();
  void update();
  Settings settings;
  Readings readings;
  bool sendReadingUpdateToApp;
  bool connectToInternetSuccessful();
  bool linkDeviceSuccess(String deviceID);

  // NVS Methods
  void restoreSettings();
  void saveInt(const char* key, int value);
  void saveFloat(const char* key, float value);
  void saveBool(const char* key, bool value);
  void saveString(const char* key, String value);
private:
  Preferences preferences;  // Create Preferences object
  String device_token;
  void sendReadingsToWebsite();
  const unsigned long websiteUpdateInterval = 900000;  // 15 mins.
  unsigned long lastWebsiteUpdate = websiteUpdateInterval;
  bool tempIsOk();
  void updateTime();
  const char* ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = -18000;  // timezone offset.
  const int daylightOffset_sec = 3600;
  unsigned long lastTimeUpdate;
  int milMin;
  int milHour;

  // hardware pins
  int _pumpPin;
  int _lightPin;
  void readSensors();
  void applyHardwareState();
};

#endif  // SMART_AQUARIUM_H