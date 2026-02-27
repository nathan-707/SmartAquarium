#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>
#include "SmartAquarium.h"

#define PIN 38  // pixels.
#define NUMPIXELS 10



// hardware pins.
#define tdsPin 2           // cs
#define tempSenPin 17       // dc
#define waterLevelPin 11    // reset
#define phSenPin 8          // SDA
#define turbiditySenPin 18  // CLK

#define pumpPin 21          // a
#define warningLightPin 15  // b
#define lastFedButton 16     // c


// void SmartAquarium::begin(int pumpPin, int tdsPin, int tempSenPin, int waterLevelPin, int phSenPin, int turSenPin, int warningLightPin, int lastFedButton)

// TODO: read temp
// TODO

//DONE::::::::::::::::::::::::::::::::
// TODO:: add code for api inside smartAquarium.h and smartAquarium.cpp to sync to website
// TODO:: add code inside smartAquarium.h and smartAquarium.cpp to sync to read sensors and set actual readings to the sensors
// TODO:: use preferences lib to store settings and wifi ssid and password across boots.

SmartAquarium aquarium;
bool testing = false;
volatile bool lampUpdated = true;


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

void pingApp() {
  Serial.println("PING");
  pCharacteristic->setValue(aquarium.serializeSettings());
  pCharacteristic->notify();
  readingsChacteristic->setValue(aquarium.serializeReadings());
  readingsChacteristic->notify();
}

void pingOnlyReadings() {
  Serial.println("READINGS");
  readingsChacteristic->setValue(aquarium.serializeReadings());
  readingsChacteristic->notify();
}

/** Server Callbacks */
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    Serial.printf("Client connected: %s\n", connInfo.getAddress().toString().c_str());
    // Update connection params for faster response
    pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 60);
    // Update value and notify
    pingApp();
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    Serial.printf("Client disconnected - start advertising\n");
    NimBLEDevice::startAdvertising();
  }
} serverCallbacks;

class IosCommandCallbacks : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* iosCommandCallbacks, NimBLEConnInfo& connInfo) override {
  }
  void onWrite(NimBLECharacteristic* iosCommandCallbacks, NimBLEConnInfo& connInfo) override {
    String payload = iosCommandCallbacks->getValue().c_str();
    StaticJsonDocument<256> doc;  // Increased size slightly
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    const char* command = doc["command"];
    const char* value_str = doc["value"];  // Used for string comparisons

    if (strcmp(command, "RGB") == 0) {
      aquarium.settings.r_LED = int(doc["value"]);
      aquarium.settings.g_LED = int(doc["value2"]);
      aquarium.settings.b_LED = int(doc["value3"]);

      // Save changes to NVS
      aquarium.saveInt("r_led", aquarium.settings.r_LED);
      aquarium.saveInt("g_led", aquarium.settings.g_LED);
      aquarium.saveInt("b_led", aquarium.settings.b_LED);
      lampUpdated = true;


    } else if (strcmp(command, "bubbler") == 0) {
      bool isOn = (strcmp(value_str, "true") == 0);
      aquarium.settings.bubbler_isOn = isOn;
      aquarium.saveBool("bubbler", isOn);  // Save
      Serial.println(isOn ? "Bubbler on!" : "Bubbler off!");

    } else if (strcmp(command, "temp_Warning_thres") == 0) {
      aquarium.settings.temp_Warning_thres = doc["value"];
      aquarium.saveFloat("temp_warn", aquarium.settings.temp_Warning_thres);  // Save
      Serial.println(aquarium.settings.temp_Warning_thres);

    } else if (strcmp(command, "targetTemp") == 0) {
      aquarium.settings.targetTemp = doc["value"];
      aquarium.saveFloat("target_temp", aquarium.settings.targetTemp);  // Save
      Serial.println(aquarium.settings.targetTemp);

    } else if (strcmp(command, "tds_Warning_thres") == 0) {
      aquarium.settings.tds_Warning_thres = doc["value"];
      aquarium.saveFloat("tds_warn", aquarium.settings.tds_Warning_thres);  // Save
      Serial.println(aquarium.settings.tds_Warning_thres);

    } else if (strcmp(command, "daysFed_Warning_thres") == 0) {
      aquarium.settings.daysFed_Warning_thres = doc["value"];
      aquarium.saveInt("fed_warn", aquarium.settings.daysFed_Warning_thres);  // Save
      Serial.println(aquarium.settings.daysFed_Warning_thres);

    } else if (strcmp(command, "lamp") == 0) {
      bool isOn = (strcmp(value_str, "true") == 0);
      aquarium.settings.lamp_isOn = isOn;
      aquarium.saveBool("lamp", isOn);  // Save
      Serial.println(isOn ? "Lamp On." : "Lamp Off.");
      lampUpdated = true;

    } else if (strcmp(command, "brightness") == 0) {
      aquarium.settings.brightness = doc["value"];
      aquarium.saveFloat("bright", aquarium.settings.brightness);  // Save
      Serial.println(aquarium.settings.brightness);
      lampUpdated = true;
    } else if (strcmp(command, "lightCycle") == 0) {
      int cycleIndex = doc["value"];
      aquarium.settings.lightCycle = static_cast<LightCycle>(cycleIndex);
      aquarium.saveInt("cycle", cycleIndex);  // Save
      Serial.printf("Light Cycle set to mode: %d\n", cycleIndex);

    } else if (strcmp(command, "onTimeHr") == 0) {
      aquarium.settings.onTimeHr = doc["value"];
      aquarium.saveInt("on_h", aquarium.settings.onTimeHr);  // Save
      Serial.println(aquarium.settings.onTimeHr);

    } else if (strcmp(command, "onTimeMin") == 0) {
      aquarium.settings.onTimeMin = doc["value"];
      aquarium.saveInt("on_m", aquarium.settings.onTimeMin);  // Save
      Serial.println(aquarium.settings.onTimeMin);

    } else if (strcmp(command, "offTimeHr") == 0) {
      aquarium.settings.offTimeHr = doc["value"];
      aquarium.saveInt("off_h", aquarium.settings.offTimeHr);  // Save
      Serial.println(aquarium.settings.offTimeHr);

    } else if (strcmp(command, "offTimeMin") == 0) {
      aquarium.settings.offTimeMin = doc["value"];
      aquarium.saveInt("off_m", aquarium.settings.offTimeMin);  // Save
      Serial.println(aquarium.settings.offTimeMin);

    } else if (strcmp(command, "ssid") == 0) {
      aquarium.settings.ssid = doc["value"].as<String>();
      aquarium.saveString("ssid", aquarium.settings.ssid);  // Save
      Serial.println("SSID Updated");

    } else if (strcmp(command, "password") == 0) {
      aquarium.settings.password = doc["value"].as<String>();
      aquarium.saveString("pass", aquarium.settings.password);  // Save
      Serial.println("Password Updated");
    }
  }
} iosCommandCallbacks;
class ReadingCommandCallbacks : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* readingCommandCallbacks, NimBLEConnInfo& connInfo) override {
  }
  void onWrite(NimBLECharacteristic* readingCommandCallbacks, NimBLEConnInfo& connInfo) override {
  }
} readingCommandCallbacks;


/** Characteristic Callbacks */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    pingApp();
  }
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    pingApp();
  }
} chrCallbacks;

void setupBLE() {
  Serial.println("Starting NimBLE Server...");
  NimBLEDevice::init("NimBLE-Server");
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);
  NimBLEService* pService = pServer->createService(SERVICE_UUID);
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

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("Smart Aquarium");
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();
  Serial.println("Advertising Started");
}
//////////////////////////////////// end of bluetooth ////////////////////////////////////////////////////


void setup() {
  Serial.begin(9600);
  setupBLE();
  pixels.begin();
  aquarium.begin(pumpPin, tdsPin, tempSenPin, waterLevelPin, phSenPin, turbiditySenPin, warningLightPin, lastFedButton);

  aquarium.restoreSettings();
  pingOnlyReadings();  // send update to app to tell it aquarium is connecting before starting.
  bool connected = aquarium.connectToInternetSuccessful();
  pingOnlyReadings();  // send update to app to tell it aquarium is connecting before starting.



  while (connected == false) {
    connected = aquarium.connectToInternetSuccessful();
    pingOnlyReadings();  // send update to app to tell it aquarium is connecting before starting.
  }

  pingApp();

  // aquarium.linkDeviceSuccess("HOKTA0"); // klklkl sense currently linked.
  // delay(5000);
}


void updateAllLights(int red, int green, int blue) {

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(green, red, blue));
  }
  pixels.show();
}



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
    RGB result = aquarium.standardLightCycle(true, simHour, simMin);
    if (result.update) {
      updateAllLights(result.red, result.green, result.blue);
    }

    // 3. Retrieve the color explicitly from the hardware/library buffer
    uint32_t packedColor = pixels.getPixelColor(0);
    uint8_t r = (packedColor >> 16) & 0xFF;
    uint8_t g = (packedColor >> 8) & 0xFF;
    uint8_t b = packedColor & 0xFF;

    // 4. Format Output: 12-Hour Time + AM/PM
    String amPm = (simHour < 12) ? "AM" : "PM";
    int displayHour = simHour % 12;
    if (displayHour == 0) displayHour = 12;  // Handle Midnight/Noon

    Serial.printf("%2d:%02d %s | %3d %3d %3d\n", displayHour, simMin, amPm.c_str(), r, g, b);
    // Speed Control
    delay(40);
  }
  Serial.println("-------------------------");
  Serial.println("--- TEST COMPLETE ---");
}

void managePixels() {

  if (aquarium.settings.lamp_isOn == false) {  // turn
    updateAllLights(0, 0, 0);
    return;
  }

  // lights are not off. follow current light cycle setting.
  if (aquarium.settings.lightCycle == LightCycle::standard) {

    if (testing) {
      runTestCycle();
    } else {

      RGB result = aquarium.standardLightCycle();

      if (result.update || lampUpdated) {
        lampUpdated = false;
        updateAllLights(result.red, result.green, result.blue);
      }
    }

  } else if (aquarium.settings.lightCycle == LightCycle::noSchedule) {
    updateAllLights(aquarium.settings.r_LED, aquarium.settings.g_LED, aquarium.settings.b_LED);
  }
}

void loop() {
  managePixels();
  aquarium.update();  // for now randomly changes the reading, later make it actually read sensors.

  // Only notify if a client is connected
  if (pServer->getConnectedCount() > 0) {
    if (aquarium.sendReadingUpdateToApp) {  // sensors just got read. report to the app then clear the flag.
      aquarium.sendReadingUpdateToApp = false;
      pingOnlyReadings();
    }
  }
}
