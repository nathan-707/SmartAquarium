#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "SmartAquarium.h"


#define pumpPin 0
#define lightPin 0

SmartAquarium aquarium(pumpPin, lightPin);
#define PIN 38
#define NUMPIXELS 1
/////////////////////////////////// mark bluetooth ///////////////////////////////////////////////////
#define SERVICE_UUID "E56A082E-C49B-47CA-A2AB-389127B8ABE7"
#define CHARACTERISTIC_UUID "FF3F"

#define CHARACTERISTIC_UUID_IOS "BB3B"
#define CHARACTERISTIC_UUID_READINGS "CC3C"


static NimBLEServer* pServer;
NimBLECharacteristic* pCharacteristic;
NimBLECharacteristic* appCommandChacteristic;
NimBLECharacteristic* readingsChacteristic;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

volatile bool updateLights = true;
bool testing = false;
int milHour = 15;
int milMin = 30;


void pingSettingsToApp() {
  pCharacteristic->setValue(aquarium.settings.serialize());
  pCharacteristic->notify();
}

/** Server Callbacks */
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    Serial.printf("Client connected: %s\n", connInfo.getAddress().toString().c_str());
    // Update connection params for faster response
    pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 60);
    // Update value and notify
    pingSettingsToApp();
    sendReadingsUpdateToApp();
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    Serial.printf("Client disconnected - start advertising\n");
    NimBLEDevice::startAdvertising();
  }
} serverCallbacks;


class IosCommandCallbacks : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* iosCommandCallbacks, NimBLEConnInfo& connInfo) override {
    Serial.printf("Read request on: %s\n", iosCommandCallbacks->getUUID().toString().c_str());
  }

  void onWrite(NimBLECharacteristic* iosCommandCallbacks, NimBLEConnInfo& connInfo) override {

    String payload = iosCommandCallbacks->getValue().c_str();
    // Stream& input;
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    const char* command = doc["command"];
    const char* value2 = doc["value2"];
    const char* value = doc["value"];
    const char* value3 = doc["value3"];


    if (strcmp(command, "RGB") == 0) {  // rgb update.
      aquarium.settings.r_LED = int(doc["value"]);
      aquarium.settings.g_LED = int(doc["value2"]);
      aquarium.settings.b_LED = int(doc["value3"]);
      updateLights = true;

    } else if (strcmp(command, "bubbler") == 0) {  // bubbler toggled.

      if (strcmp(value, "true") == 0) {
        aquarium.settings.bubbler_isOn = true;
      } else {
        aquarium.settings.bubbler_isOn = false;
      }

      if (aquarium.settings.bubbler_isOn) {
        Serial.println("Bubbler on!");
      } else {
        Serial.println("Bubbler off!");
      }
    }

    else if (strcmp(command, "temp_Warning_thres") == 0) {  // temp_Warning_thres
      aquarium.settings.temp_Warning_thres = doc["value"];
      Serial.print("temp_Warning_thres: ");
      Serial.println(aquarium.settings.temp_Warning_thres);
    }

    else if (strcmp(command, "targetTemp") == 0) {  // targetTemp
      aquarium.settings.targetTemp = doc["value"];
      Serial.print("targetTemp: ");
      Serial.println(aquarium.settings.targetTemp);
    }

    else if (strcmp(command, "tds_Warning_thres") == 0) {  // tds_Warning_thres
      aquarium.settings.tds_Warning_thres = doc["value"];
      Serial.print("tds_Warning_thres: ");
      Serial.println(aquarium.settings.tds_Warning_thres);
    }

    else if (strcmp(command, "daysFed_Warning_thres") == 0) {  // daysFed_Warning_thres
      aquarium.settings.daysFed_Warning_thres = doc["value"];
      Serial.print("daysFed_Warning_thres: ");
      Serial.println(aquarium.settings.daysFed_Warning_thres);
    }

    else if (strcmp(command, "lamp") == 0) {  // lamp
      if (strcmp(value, "true") == 0) {
        aquarium.settings.lamp_isOn = true;
      } else {
        aquarium.settings.lamp_isOn = false;
      }

      if (aquarium.settings.lamp_isOn) {
        Serial.println("Lamp On.");
      } else {
        Serial.println("Lamp Off.");
      }
      updateLights = true;
    }

    else if (strcmp(command, "brightness") == 0) {  // brightness
      aquarium.settings.brightness = doc["value"];
      updateLights = true;
      Serial.print("Brightness set to: ");
      Serial.println(aquarium.settings.brightness);
    }

    else if (strcmp(command, "lightCycle") == 0) {  // lightCycle (Enum)
      // Cast the integer value from JSON back to the LightCycle Enum
      int cycleIndex = doc["value"];
      aquarium.settings.lightCycle = static_cast<LightCycle>(cycleIndex);
      updateLights = true;
      Serial.print("Light Cycle set to mode: ");
      Serial.println(cycleIndex);
    }

    else if (strcmp(command, "onTimeHr") == 0) {  // onTimeHr
      aquarium.settings.onTimeHr = doc["value"];
      updateLights = true;  // Time change requires re-check of light state
      Serial.print("On Time Hour: ");
      Serial.println(aquarium.settings.onTimeHr);
    }

    else if (strcmp(command, "onTimeMin") == 0) {  // onTimeMin
      aquarium.settings.onTimeMin = doc["value"];
      updateLights = true;
      Serial.print("On Time Min: ");
      Serial.println(aquarium.settings.onTimeMin);
    }

    else if (strcmp(command, "offTimeHr") == 0) {  // offTimeHr
      aquarium.settings.offTimeHr = doc["value"];
      updateLights = true;
      Serial.print("Off Time Hour: ");
      Serial.println(aquarium.settings.offTimeHr);
    }

    else if (strcmp(command, "offTimeMin") == 0) {  // offTimeMin
      aquarium.settings.offTimeMin = doc["value"];
      updateLights = true;
      Serial.print("Off Time Min: ");
      Serial.println(aquarium.settings.offTimeMin);
    }



    // else if ... other commands to read.
  }

} iosCommandCallbacks;


class ReadingCommandCallbacks : public NimBLECharacteristicCallbacks {

  void onRead(NimBLECharacteristic* readingCommandCallbacks, NimBLEConnInfo& connInfo) override {
    Serial.printf("Read request on: %s\n", readingCommandCallbacks->getUUID().toString().c_str());
  }

  void onWrite(NimBLECharacteristic* readingCommandCallbacks, NimBLEConnInfo& connInfo) override {
  }

} readingCommandCallbacks;


/** Characteristic Callbacks */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {

  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    Serial.printf("Read request on: %s\n", pCharacteristic->getUUID().toString().c_str());
    pingSettingsToApp();
  }

  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    pingSettingsToApp();
  }

} chrCallbacks;

void setupBLE() {
  Serial.println("Starting NimBLE Server...");

  /** 1. Initialize NimBLE */
  NimBLEDevice::init("NimBLE-Server");

  /** 2. Create Server */
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);

  /** 3. Create the Service */
  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  /** 4. Create the Characteristic (FF3F) & Assign to GLOBAL variable */
  // We assign the result to 'pCharacteristic' which is declared at the top of the file
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pCharacteristic->setValue("1");
  pCharacteristic->setCallbacks(&chrCallbacks);


  appCommandChacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_IOS,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  appCommandChacteristic->setCallbacks(&iosCommandCallbacks);


  readingsChacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_READINGS,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  readingsChacteristic->setCallbacks(&readingCommandCallbacks);

  /** 5. Start the Service */
  pService->start();

  /** 6. Start Advertising */
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("Smart Aquarium");
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();

  Serial.println("Advertising Started");
}
//////////////////////////////////// end of bluetooth ////////////////////////////////////////////////////

void setup(void) {
  Serial.begin(9600);



  // TODO:: Connect to internet.
  // TODO:: set 'milHour' and 'milMin' to the actual time.
  // TODO:: schdule timer to update milHour and milMin every 30 seconds or so.
  // TODO:: add code for api inside smartAquarium.h and smartAquarium.cpp to sync to website
  // TODO:: add code inside smartAquarium.h and smartAquarium.cpp to sync to read sensors and set actual readings to the sensors



  pixels.begin();
  setupBLE();
}
int value;

bool tempIsOk() {
  if (aquarium.readings.water_temp < (aquarium.settings.targetTemp - aquarium.settings.temp_Warning_thres)
      || aquarium.readings.water_temp > (aquarium.settings.targetTemp + aquarium.settings.temp_Warning_thres)) {
    return false;
  } else {
    return true;
  }
}


void sendReadingsUpdateToApp() {
  // String serialize(bool tds_isOk, bool temp_isOk, bool daysFed_isOk, bool waterLevel_isOk) {

  readingsChacteristic->setValue(aquarium.readings.serialize(
    aquarium.readings.tds_level < aquarium.settings.tds_Warning_thres ? true : false,
    tempIsOk(),
    aquarium.readings.daysSinceFed <= aquarium.settings.daysFed_Warning_thres ? true : false,
    aquarium.readings.waterLevel_isFull));
  readingsChacteristic->notify();
}

// Assuming you are using NeoPixel or a similar RGB library
// #include <Adafruit_NeoPixel.h>

// Assumes you have: #include <Adafruit_NeoPixel.h>
// Assumes 'pixels' is globally defined




void runTestCycle() {



  Serial.println("\n--- STARTING 24H TURBO TEST ---");
  Serial.printf("Schedule: ON %02d:%02d | OFF %02d:%02d\n", aquarium.settings.onTimeHr, aquarium.settings.onTimeMin, aquarium.settings.offTimeHr, aquarium.settings.offTimeMin);
  Serial.println("TIME      | R   G   B");
  Serial.println("-------------------------");

  // Loop through every minute of the day (0 to 1439)
  for (int i = 0; i < 1440; i++) {

    // 1. Calculate Time
    int simHour = i / 60;
    int simMin = i % 60;

    // 2. Run the Cycle Logic (Force update with testMode = true)
    // Ensure you updated standardLightCycle to accept the 'true' argument!




    RGB result = aquarium.standardLightCycle(simHour, simMin, aquarium.settings.onTimeHr, aquarium.settings.onTimeMin, aquarium.settings.offTimeHr, aquarium.settings.offTimeMin, true);

    if (result.update) {
      pixels.setPixelColor(0, pixels.Color(result.green, result.red, result.blue));
      pixels.show();
    }

    // 3. Retrieve the color explicitly from the hardware/library buffer
    // This lets us see exactly what the function decided to do
    uint32_t packedColor = pixels.getPixelColor(0);
    uint8_t r = (packedColor >> 16) & 0xFF;
    uint8_t g = (packedColor >> 8) & 0xFF;
    uint8_t b = packedColor & 0xFF;

    // 4. Format Output: 12-Hour Time + AM/PM
    String amPm = (simHour < 12) ? "AM" : "PM";
    int displayHour = simHour % 12;
    if (displayHour == 0) displayHour = 12;  // Handle Midnight/Noon

    // Print: "10:30 AM | 255 120 0"
    // %2d ensures single digit hours align nicely
    Serial.printf("%2d:%02d %s | %3d %3d %3d\n", displayHour, simMin, amPm.c_str(), r, g, b);

    // Speed Control (Total test time ~28 seconds)
    delay(40);
  }

  Serial.println("-------------------------");
  Serial.println("--- TEST COMPLETE ---");
}

void managePixels() {

  if (updateLights) {
    updateLights = false;

    if (aquarium.settings.lamp_isOn == false) {  // turn
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      return;
    }

    // lights are not off. follow current light cycle setting.
    if (aquarium.settings.lightCycle == LightCycle::standard) {

      if (testing) {
        runTestCycle();
      } else {



        RGB result = aquarium.standardLightCycle(milHour, milMin, aquarium.settings.onTimeHr, aquarium.settings.onTimeMin, aquarium.settings.offTimeHr, aquarium.settings.offTimeMin, true);

        if (result.update) {
          pixels.setPixelColor(0, pixels.Color(result.green, result.red, result.blue));
          pixels.show();
        }
      }


    } else if (aquarium.settings.lightCycle == LightCycle::noSchedule) {
      pixels.setPixelColor(0, pixels.Color(aquarium.settings.g_LED, aquarium.settings.r_LED, aquarium.settings.b_LED));
      pixels.show();
    }
  }
}

void loop() {

  managePixels();
  aquarium.update();  // for now randomly changes the reading, later make it actually read sensors.

  // Only notify if a client is connected
  if (pServer->getConnectedCount() > 0) {
    if (aquarium.sendReadingUpdateToApp) {  // sensors just got read. report to the app then clear the flag.
      aquarium.sendReadingUpdateToApp = false;
      sendReadingsUpdateToApp();
    }
  }
}
