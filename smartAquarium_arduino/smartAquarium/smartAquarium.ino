

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
    Serial.printf("Write request on: %s, Value: %s\n",
                  iosCommandCallbacks->getUUID().toString().c_str(),
                  iosCommandCallbacks->getValue().c_str());

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
      pixels.setPixelColor(0, pixels.Color(aquarium.settings.g_LED, aquarium.settings.r_LED, aquarium.settings.b_LED));
      pixels.show();
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
    }



    // else if ... other commands to read.
  }

} iosCommandCallbacks;


class ReadingCommandCallbacks : public NimBLECharacteristicCallbacks {

  void onRead(NimBLECharacteristic* readingCommandCallbacks, NimBLEConnInfo& connInfo) override {
    Serial.printf("Read request on: %s\n", readingCommandCallbacks->getUUID().toString().c_str());
  }

  void onWrite(NimBLECharacteristic* readingCommandCallbacks, NimBLEConnInfo& connInfo) override {
    Serial.printf("Write request on: %s, Value: %s\n",
                  readingCommandCallbacks->getUUID().toString().c_str(),
                  readingCommandCallbacks->getValue().c_str());
  }

} readingCommandCallbacks;


/** Characteristic Callbacks */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {

  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    Serial.printf("Read request on: %s\n", pCharacteristic->getUUID().toString().c_str());
    pingSettingsToApp();
  }

  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    Serial.printf("Write request on: %s, Value: %s\n",
                  pCharacteristic->getUUID().toString().c_str(),
                  pCharacteristic->getValue().c_str());
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
  pAdvertising->setName("Accessory");
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();

  Serial.println("Advertising Started");
}
//////////////////////////////////// end of bluetooth ////////////////////////////////////////////////////

void setup(void) {
  Serial.begin(9600);
  pixels.begin();
  setupBLE();
}
int value;


void sendReadingsUpdateToApp() {
  // String serialize(bool tds_isOk, bool temp_isOk, bool daysFed_isOk, bool waterLevel_isOk) {

  readingsChacteristic->setValue(aquarium.readings.serialize(
    aquarium.readings.tds_level < aquarium.settings.tds_Warning_thres ? true : false,
    aquarium.readings.water_temp < aquarium.settings.temp_Warning_thres ? true : false,
    aquarium.readings.daysSinceFed <= aquarium.settings.daysFed_Warning_thres ? true : false,
    aquarium.readings.waterLevel_isFull));
  readingsChacteristic->notify();
}

void loop() {

  delay(2000);
  aquarium.update();  // for now randomly changes the reading, later make it actually read sensors.

  // Only notify if a client is connected
  if (pServer->getConnectedCount() > 0) {

    if (readingsChacteristic) {
      sendReadingsUpdateToApp();
    }
  }
}