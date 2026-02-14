#include <stdlib.h>
#include "SmartAquarium.h"
#include <Adafruit_NeoPixel.h>


// -------------------------------------------------------------------------
// Constructor & Setup
// -------------------------------------------------------------------------

SmartAquarium::SmartAquarium(int pumpPin, int lightPin) {
  _pumpPin = pumpPin;
  _lightPin = lightPin;

  // Set Defaults
  settings.bubbler_isOn = false;
  settings.r_LED = 0;
  settings.g_LED = 255;  // Start Green
  settings.b_LED = 0;

  readings.tds_level = 0.0;
  readings.water_temp = 0.0;
  readings.daysSinceFed = 0;
}

void SmartAquarium::begin() {
  pinMode(_pumpPin, OUTPUT);
  pinMode(_lightPin, OUTPUT);

  // Apply initial state
  applyHardwareState();
}


void SmartAquarium::update() {
  readSensors();
  applyHardwareState();
}


void SmartAquarium::readSensors() {
  static unsigned long lastReadTime = 0;

  // Read sensors every 2 seconds (Non-blocking)
  if (millis() - lastReadTime > 2000) {
    lastReadTime = millis();
    // --- SIMULATION DATA (Replace with real sensors) ---
    readings.water_temp = 70 + (random(-5, 5));  // 23.0 - 25.0
    readings.tds_level = 500 + random(-10, 10);

    bool randomLevelReading = true;
    readings.waterLevel_isFull = randomLevelReading;

    // flag update to app.
    sendReadingUpdateToApp = true;
  }
}

void SmartAquarium::applyHardwareState() {
  // 1. Control Bubbler
  digitalWrite(_pumpPin, settings.bubbler_isOn ? HIGH : LOW);
}


RGB SmartAquarium::standardLightCycle(int currentHour, int currentMin, int userOnHour, int userOnMin, int userOffHour, int userOffMin, bool testMode) {

  // --- STATE TRACKING ---
  // We keep static variables to know what the previous state was
  static int lastR = -1, lastG = -1, lastB = -1;

  // CONSTANTS
  const int rampDuration = 30;
  const float halfDuration = 15.0;

  // DEFINITION: The "Target" color for Sunrise/Sunset (Deep Orange)
  const int sunR = 255;
  const int sunG = 60;
  const int sunB = 0;

  // --- 1. CONVERT TO MINUTES ---
  int currentTotalMin = (currentHour * 60) + currentMin;
  int onTimeTotalMin = (userOnHour * 60) + userOnMin;
  int offTimeTotalMin = (userOffHour * 60) + userOffMin;

  // --- 2. HANDLE MIDNIGHT WRAPAROUND ---
  if (offTimeTotalMin < onTimeTotalMin) {
    offTimeTotalMin += 1440;
    if (currentTotalMin < onTimeTotalMin) {
      currentTotalMin += 1440;
    }
  }

  float r = 0, g = 0, b = 0;

  // --- 3. DETERMINE PHASE ---

  // A. NIGHT (Before Sunrise)
  if (currentTotalMin < onTimeTotalMin) {
    r = 0;
    g = 0;
    b = 0;
  }

  // B. SUNRISE (Split Ramp)
  else if (currentTotalMin < (onTimeTotalMin + rampDuration)) {
    float elapsed = currentTotalMin - onTimeTotalMin;

    // Phase 1: Black -> Orange
    if (elapsed < halfDuration) {
      float progress = elapsed / halfDuration;  // 0.0 -> 1.0
      r = sunR * progress;
      g = sunG * progress;
      b = sunB * progress;
    }
    // Phase 2: Orange -> Custom Color
    else {
      float progress = (elapsed - halfDuration) / halfDuration;  // 0.0 -> 1.0
      r = sunR + ((settings.r_LED - sunR) * progress);
      g = sunG + ((settings.g_LED - sunG) * progress);
      b = sunB + ((settings.b_LED - sunB) * progress);
    }
  }

  // C. DAYLIGHT (Steady State)
  else if (currentTotalMin < (offTimeTotalMin - rampDuration)) {
    r = settings.r_LED;
    g = settings.g_LED;
    b = settings.b_LED;
  }

  // D. SUNSET (Split Ramp)
  else if (currentTotalMin < offTimeTotalMin) {
    float timeRemaining = offTimeTotalMin - currentTotalMin;

    // Phase 1: Custom Color -> Orange (First 15 mins of Sunset)
    if (timeRemaining > halfDuration) {
      float elapsedInSunset = rampDuration - timeRemaining;
      float progress = elapsedInSunset / halfDuration;  // 0.0 -> 1.0

      r = settings.r_LED + ((sunR - settings.r_LED) * progress);
      g = settings.g_LED + ((sunG - settings.g_LED) * progress);
      b = settings.b_LED + ((sunB - settings.b_LED) * progress);
    }
    // Phase 2: Orange -> Black (Last 15 mins of Sunset)
    else {
      float progress = timeRemaining / halfDuration;  // 1.0 -> 0.0
      r = sunR * progress;
      g = sunG * progress;
      b = sunB * progress;
    }
  }

  // E. NIGHT (After Sunset)
  else {
    r = 0;
    g = 0;
    b = 0;
  }

  // --- 4. PREPARE OUTPUT ---
  RGB output;
  output.red = (int)r;
  output.green = (int)g;
  output.blue = (int)b;
  output.update = false;

  // --- 5. CHECK FOR CHANGES ---
  // Only flag update=true if values changed OR we are forcing a test
  if (testMode || output.red != lastR || output.green != lastG || output.blue != lastB) {
    output.update = true;

    // Update static history
    lastR = output.red;
    lastG = output.green;
    lastB = output.blue;
  }

  return output;
}


