#include "esp32-hal-gpio.h"
#include "ArduinoJson.hpp"
#include "HardwareSerial.h"
#include "esp32-hal.h"
#include <stdlib.h>
#include "SmartAquarium.h"
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

bool SmartAquarium::linkDeviceSuccess(String deviceID) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: WiFi not connected");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  if (!http.begin(client, "https://aquasense.replit.app/create_device")) {
    Serial.println("Connection failed!");
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  DynamicJsonDocument doc(512);
  doc["device_id"] = deviceID;
  String requestBody;
  serializeJson(doc, requestBody);
  Serial.print("Sending Payload: ");
  Serial.println(requestBody);
  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Server: " + response);
    DynamicJsonDocument resDoc(1024);
    deserializeJson(resDoc, response);
    if (resDoc["success"]) {
      device_token = resDoc["device_id"].as<String>();
      saveString("device", device_token);
      http.end();
      Serial.print("Device linked success. saved key: ");
      Serial.println(device_token);
      return true;
    }
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  Serial.println("Device linking failed.");
  return false;
}

bool SmartAquarium::tempIsOk() {
  if (readings.water_temp < (settings.targetTemp - settings.temp_Warning_thres)
      || readings.water_temp > (settings.targetTemp + settings.temp_Warning_thres)) {
    return false;
  } else {
    return true;
  }
}

SmartAquarium::SmartAquarium() {

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
  device_token = preferences.getString("device");
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
  Serial.printf("token: %s\n", device_token);
  Serial.println(device_token);

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

void SmartAquarium::begin(int pumpPin, int tdsPin, int tempSenPin, int waterLevelPin, int phSenPin, int turSenPin, int warningLightPin, int lastFedButton) {
  Serial.println("Smart Aquarium begin!");

  _pumpPin = pumpPin;
  pinMode(_pumpPin, OUTPUT);

  _tdsPin = tdsPin;
  pinMode(_tdsPin, INPUT);
  Serial.println("TDS: ");
  Serial.println(_tdsPin);

  _tempSenPin = tempSenPin;
  pinMode(_tempSenPin, INPUT);

  _waterLevelPin = waterLevelPin;
  pinMode(_waterLevelPin, INPUT);

  _phSenPin = phSenPin;
  pinMode(_phSenPin, INPUT);

  _turSenPin = turSenPin;
  pinMode(_turSenPin, INPUT);

  _warningLightPin = warningLightPin;
  pinMode(_warningLightPin, OUTPUT);

  _lastFedButton = lastFedButton;
  pinMode(_lastFedButton, INPUT_PULLUP);

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

  // --- ADDED: Fast TDS Sampling Background Task ---
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {  // every 40 milliseconds
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(_tdsPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }
  // ------------------------------------------------

  readSensors();
  applyHardwareState();
}


void SmartAquarium::sendReadingsToWebsite() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: WiFi not connected");
    return;
  }

  Serial.println("Uploading data to Aquasense...");
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  if (!http.begin(client, "https://aquasense.replit.app/upload_data")) {
    Serial.println("Connection failed!");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  DynamicJsonDocument doc(2048);
  doc["device_token"] = device_token;
  JsonArray dataArray = doc.createNestedArray("data");
  long now = time(nullptr);

  auto addDataPoint = [&](const char* key, float value) {
    JsonObject point = dataArray.createNestedObject();
    point["type"] = key;
    point["value"] = value;
    point["time"] = now;
  };

  addDataPoint("tds", readings.tds_level);
  addDataPoint("temp", readings.water_temp);
  // addDataPoint("fed", readings.daysSinceFed);  // 0 or 1
  addDataPoint("turbidity", readings.turbidity);
  addDataPoint("pH", readings.pH);
  addDataPoint("waterLevel_isOk", readings.waterLevel_isFull);

  String requestBody;
  serializeJson(doc, requestBody);
  Serial.println("Payload: " + requestBody);
  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Server Response: ");
    Serial.println(response);
  } else {
    Serial.print("Error sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}


int SmartAquarium::getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}



float SmartAquarium::readTDS() {
  // 1. Copy the background buffer for safe filtering
  for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
    analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
  }
  
  // 2. Get the median voltage
  averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0;
  
  // 3. Convert current water temp from Fahrenheit to Celsius for the math
  float tempC = (readings.water_temp - 32.0) * 5.0 / 9.0;
  float compensationCoefficient = 1.0 + 0.02 * (tempC - 25.0);
  
  // 4. Calculate final TDS
  float compensationVoltage = averageVoltage / compensationCoefficient;
  tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
  
  Serial.print("TDS Value: ");
  Serial.print(tdsValue, 0);
  Serial.println(" ppm");
  
  return tdsValue;
}

void SmartAquarium::readSensors() {
  static unsigned long lastReadTime = 0;
  static bool poweredUp = true;

  if (millis() - lastReadTime > 2000 || poweredUp) {  // update app every 2 seconds
    lastReadTime = millis();
    poweredUp = false;

    // todo: actually read these and set them.
    readings.waterLevel_isFull = true;
    readings.turbidity = 1;
    readings.pH = 1;
    readings.water_temp = 70;


    readings.tds_level = readTDS();
    // todo: get camera snapshot
    sendReadingUpdateToApp = true;
  }

  if (millis() - lastWebsiteUpdate > websiteUpdateInterval) {  // update website every 15 mins
    lastWebsiteUpdate = millis();
    sendReadingsToWebsite();
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