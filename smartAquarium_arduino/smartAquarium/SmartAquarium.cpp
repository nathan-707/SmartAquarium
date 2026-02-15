#include "HardwareSerial.h"
#include "esp32-hal.h"
#include <stdlib.h>
#include "SmartAquarium.h"
#include <Adafruit_NeoPixel.h>

bool SmartAquarium::tempIsOk() {
  if (readings.water_temp < (settings.targetTemp - settings.temp_Warning_thres)
      || readings.water_temp > (settings.targetTemp + settings.temp_Warning_thres)) {
    return false;
  } else {
    return true;
  }
}


SmartAquarium::SmartAquarium(int pumpPin, int lightPin) {
  _pumpPin = pumpPin;
  _lightPin = lightPin;

  // Set Defaults (Will be overwritten by restoreSettings if NVS data exists)
  settings.bubbler_isOn = false;
  settings.r_LED = 0;
  settings.g_LED = 255;
  settings.b_LED = 0;
  readings.tds_level = 0.0;
  readings.water_temp = 0.0;
  readings.daysSinceFed = 0;
}

String SmartAquarium::serializeSettings() {
  StaticJsonDocument<400> doc;
  doc["bubbler"] = settings.bubbler_isOn;
  doc["lamp"] = settings.lamp_isOn;
  doc["temp_Warning_thres"] = settings.temp_Warning_thres;
  doc["targetTemp"] = settings.targetTemp;
  doc["tds_Warning_thres"] = settings.tds_Warning_thres;
  doc["daysFed_Warning_thres"] = settings.daysFed_Warning_thres;
  doc["r"] = settings.r_LED;
  doc["g"] = settings.g_LED;
  doc["b"] = settings.b_LED;
  doc["brightness"] = settings.brightness;
  doc["lightCycle"] = (int)settings.lightCycle;
  doc["onTimeHr"] = settings.onTimeHr;
  doc["onTimeMin"] = settings.onTimeMin;
  doc["offTimeHr"] = settings.offTimeHr;
  doc["offTimeMin"] = settings.offTimeMin;
  doc["ssid"] = settings.ssid;
  doc["password"] = settings.password;
  String output;
  serializeJson(doc, output);
  return output;
}

String SmartAquarium::serializeReadings() {
  StaticJsonDocument<400> doc;
  doc["tds"] = serialized(String(readings.tds_level, 2));
  doc["temp"] = serialized(String(readings.water_temp, 2));
  doc["fed"] = readings.daysSinceFed;
  doc["tds_isOk"] = readings.tds_level < settings.tds_Warning_thres ? true : false;
  doc["temp_isOk"] = tempIsOk();
  doc["daysFed_isOk"] = readings.daysSinceFed <= settings.daysFed_Warning_thres ? true : false;
  doc["waterLevel_isOk"] = readings.waterLevel_isFull;
  doc["lights_isOn"] = readings.lights_isOn;
  doc["turbidity"] = readings.turbidity;
  doc["pH"] = readings.pH;
  doc["status"] = (int)readings.status;

  String output;
  serializeJson(doc, output);
  return output;
}

// --- NVS IMPLEMENTATION START ---

void SmartAquarium::restoreSettings() {
  preferences.begin("aqua", true);  // Open namespace "aqua" in read-only mode

  settings.bubbler_isOn = preferences.getBool("bubbler", false);
  settings.lamp_isOn = preferences.getBool("lamp", true);

  settings.temp_Warning_thres = preferences.getFloat("temp_warn", 3.0);
  settings.targetTemp = preferences.getFloat("target_temp", 71.0);
  settings.tds_Warning_thres = preferences.getFloat("tds_warn", 500.0);
  settings.daysFed_Warning_thres = preferences.getInt("fed_warn", 1);

  settings.r_LED = preferences.getInt("r_led", 0);
  settings.g_LED = preferences.getInt("g_led", 255);
  settings.b_LED = preferences.getInt("b_led", 0);
  settings.brightness = preferences.getFloat("bright", 1.0);

  settings.lightCycle = static_cast<LightCycle>(preferences.getInt("cycle", 0));

  settings.onTimeHr = preferences.getInt("on_h", 7);
  settings.onTimeMin = preferences.getInt("on_m", 0);
  settings.offTimeHr = preferences.getInt("off_h", 24);
  settings.offTimeMin = preferences.getInt("off_m", 0);

  settings.ssid = preferences.getString("ssid", "");
  settings.password = preferences.getString("pass", "");

  preferences.end();

  // Print all restored values
  Serial.println("\n--- Settings Restored from NVS ---");
  Serial.printf("Bubbler: %s\n", settings.bubbler_isOn ? "ON" : "OFF");
  Serial.printf("Lamp: %s\n", settings.lamp_isOn ? "ON" : "OFF");
  Serial.printf("Temp Warn Thres: %.2f\n", settings.temp_Warning_thres);
  Serial.printf("Target Temp: %.2f\n", settings.targetTemp);
  Serial.printf("TDS Warn Thres: %.2f\n", settings.tds_Warning_thres);
  Serial.printf("Days Fed Warn Thres: %d\n", settings.daysFed_Warning_thres);
  Serial.printf("RGB: (%d, %d, %d)\n", settings.r_LED, settings.g_LED, settings.b_LED);
  Serial.printf("Brightness: %.2f\n", settings.brightness);
  Serial.printf("Light Cycle Mode: %d\n", (int)settings.lightCycle);
  Serial.printf("On Time: %02d:%02d\n", settings.onTimeHr, settings.onTimeMin);
  Serial.printf("Off Time: %02d:%02d\n", settings.offTimeHr, settings.offTimeMin);
  Serial.printf("SSID: %s\n", settings.ssid.c_str());
  // Don't print the password for security, or mask it
  Serial.printf("Password: %s\n", settings.password.length() > 0 ? "***" : "Empty");
  Serial.println("----------------------------------");
}

void SmartAquarium::saveInt(const char* key, int value) {
  preferences.begin("aqua", false);  // Read-Write
  preferences.putInt(key, value);
  preferences.end();
}

void SmartAquarium::saveFloat(const char* key, float value) {
  preferences.begin("aqua", false);
  preferences.putFloat(key, value);
  preferences.end();
}

void SmartAquarium::saveBool(const char* key, bool value) {
  preferences.begin("aqua", false);
  preferences.putBool(key, value);
  preferences.end();
}

void SmartAquarium::saveString(const char* key, String value) {
  preferences.begin("aqua", false);
  preferences.putString(key, value);
  preferences.end();
}
// --- NVS IMPLEMENTATION END ---

bool SmartAquarium::connectToInternetSuccessful() {
  readings.status = WifiStatus::connecting;
  Serial.print("Connecting to WiFi: ");
  Serial.println(settings.ssid.c_str());

  if (settings.ssid == "") {
    Serial.println("No SSID configured.");
    return false;
  }

  WiFi.begin(settings.ssid.c_str(), settings.password.c_str());
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
    delay(500);
    Serial.print(".");
    retryCount++;
  }
  delay(500);

  if (WiFi.status() == WL_CONNECTED) {
    readings.status = WifiStatus::connected;
    sendReadingUpdateToApp = true;
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // --- NEW: SAVE CREDENTIALS ON SUCCESS ---
    // This ensures we only persist credentials that actually work
    saveString("ssid", settings.ssid);
    saveString("pass", settings.password);
    Serial.println("WiFi Credentials Saved to NVS.");
    // ----------------------------------------

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    return true;
  } else {
    Serial.println("\nFailed to connect to WiFi. Running offline mode.");
    return false;
  }
}
void SmartAquarium::begin() {
  Serial.println("Smart Aquarium begin!");
  pinMode(_pumpPin, OUTPUT);
  pinMode(_lightPin, OUTPUT);
  update();
  updateTime();
}

void SmartAquarium::updateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Serial.println("Failed to obtain time"); // Reduced noise
    return;
  }
  milHour = timeinfo.tm_hour;
  milMin = timeinfo.tm_min;

  // Reconnect logic
  if (WiFi.status() != WL_CONNECTED && settings.ssid != "") {
    // Optional: Add logic here to retry connection occasionally, not every loop
  }
}

void SmartAquarium::update() {

  // if wifi disconnects, try to reconnect. and update wifi status.
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    readings.status = WifiStatus::wifiError;
  } else {
    readings.status = WifiStatus::connected;
  }


  if (millis() - lastTimeUpdate > 30000) {
    lastTimeUpdate = millis();
    updateTime();
  }
  readSensors();
  applyHardwareState();
}

void SmartAquarium::readSensors() {
  static unsigned long lastReadTime = 0;
  if (millis() - lastReadTime > 2000) {
    lastReadTime = millis();
    readings.water_temp = 70 + (random(-5, 5));
    readings.tds_level = 500 + random(-10, 10);
    readings.waterLevel_isFull = true;
    sendReadingUpdateToApp = true;
  }
}

void SmartAquarium::applyHardwareState() {
  digitalWrite(_pumpPin, settings.bubbler_isOn ? HIGH : LOW);
}

RGB SmartAquarium::standardLightCycle(bool testMode, int testHr, int testMin) {
  static int lastR = -1, lastG = -1, lastB = -1;
  const int rampDuration = 30;
  const float halfDuration = 15.0;
  const int sunR = 255;
  const int sunG = 60;
  const int sunB = 0;

  int currentTotalMin = ((testMode ? testHr : milHour) * 60) + (testMode ? testMin : milMin);
  int onTimeTotalMin = (settings.onTimeHr * 60) + settings.onTimeMin;
  int offTimeTotalMin = (settings.offTimeHr * 60) + settings.offTimeMin;

  if (offTimeTotalMin < onTimeTotalMin) {
    offTimeTotalMin += 1440;
    if (currentTotalMin < onTimeTotalMin) {
      currentTotalMin += 1440;
    }
  }

  float r = 0, g = 0, b = 0;

  if (currentTotalMin < onTimeTotalMin) {
    r = 0;
    g = 0;
    b = 0;
  } else if (currentTotalMin < (onTimeTotalMin + rampDuration)) {
    float elapsed = currentTotalMin - onTimeTotalMin;
    if (elapsed < halfDuration) {
      float progress = elapsed / halfDuration;
      r = sunR * progress;
      g = sunG * progress;
      b = sunB * progress;
    } else {
      float progress = (elapsed - halfDuration) / halfDuration;
      r = sunR + ((settings.r_LED - sunR) * progress);
      g = sunG + ((settings.g_LED - sunG) * progress);
      b = sunB + ((settings.b_LED - sunB) * progress);
    }
  } else if (currentTotalMin < (offTimeTotalMin - rampDuration)) {
    r = settings.r_LED;
    g = settings.g_LED;
    b = settings.b_LED;
  } else if (currentTotalMin < offTimeTotalMin) {
    float timeRemaining = offTimeTotalMin - currentTotalMin;
    if (timeRemaining > halfDuration) {
      float elapsedInSunset = rampDuration - timeRemaining;
      float progress = elapsedInSunset / halfDuration;
      r = settings.r_LED + ((sunR - settings.r_LED) * progress);
      g = settings.g_LED + ((sunG - settings.g_LED) * progress);
      b = settings.b_LED + ((sunB - settings.b_LED) * progress);
    } else {
      float progress = timeRemaining / halfDuration;
      r = sunR * progress;
      g = sunG * progress;
      b = sunB * progress;
    }
  } else {
    r = 0;
    g = 0;
    b = 0;
  }

  RGB output;
  output.red = (int)r;
  output.green = (int)g;
  output.blue = (int)b;
  output.update = false;

  if (testMode || output.red != lastR || output.green != lastG || output.blue != lastB) {
    output.update = true;
    lastR = output.red;
    lastG = output.green;
    lastB = output.blue;
  }

  return output;
}