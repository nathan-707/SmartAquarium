

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include "SmartAquarium.h"


#define pumpPin 0
#define lightPin 0

SmartAquarium aquarium(pumpPin, lightPin);

/////////////////////////////////// mark bluetooth ///////////////////////////////////////////////////
#define SERVICE_UUID "E56A082E-C49B-47CA-A2AB-389127B8ABE7"
#define CHARACTERISTIC_UUID "FF3F"

#define CHARACTERISTIC_UUID_IOS "BB3B"

static NimBLEServer* pServer;
NimBLECharacteristic* pCharacteristic;
NimBLECharacteristic* appCommandChacteristic;

/** Server Callbacks */
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    Serial.printf("Client connected: %s\n", connInfo.getAddress().toString().c_str());
    // Update connection params for faster response
    pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 60);
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
    Serial.println("RawCommand:");
    Serial.println(payload);
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
    }
  }

} iosCommandCallbacks;





/** Characteristic Callbacks */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {

  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    Serial.printf("Read request on: %s\n", pCharacteristic->getUUID().toString().c_str());
  }

  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    Serial.printf("Write request on: %s, Value: %s\n",
                  pCharacteristic->getUUID().toString().c_str(),
                  pCharacteristic->getValue().c_str());
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

  appCommandChacteristic->setValue("sugma");
  appCommandChacteristic->setCallbacks(&iosCommandCallbacks);



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
  setupBLE();
}

int val = 1;

void loop() {

  delay(2000);

  // Only notify if a client is connected
  if (pServer->getConnectedCount() > 0) {

    // DIRECT ACCESS: No more searching for UUIDs!
    // We simply use the global pointer we created in setup.
    if (pCharacteristic) {

      // Generate a value to send

      val++;

      if (val > 6) {
        val = 1;
      }
      // aquarium.settings.r_LED = val;


      std::string msg = std::to_string(val);

      // Update value and notify
      pCharacteristic->setValue(aquarium.settings.serialize());
      pCharacteristic->notify();

      Serial.printf("Notification sent: %s\n", msg.c_str());
    }
  }
}