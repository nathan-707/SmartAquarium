#include "SmartAquarium.h"

// -------------------------------------------------------------------------
// Constructor & Setup
// -------------------------------------------------------------------------

SmartAquarium::SmartAquarium(int pumpPin, int lightPin) {
  _pumpPin = pumpPin;
  _lightPin = lightPin;

  // Set Defaults
  settings.bubbler_isOn = false;
  settings.display_isOn = true;
  settings.r_LED = 0;
  settings.g_LED = 255; // Start Green
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

// -------------------------------------------------------------------------
// Main Loop Logic
// -------------------------------------------------------------------------

void SmartAquarium::update() {
  // 1. Update Sensors
  readSensors();

  // 2. Apply Settings to Hardware
  applyHardwareState();
}

// -------------------------------------------------------------------------
// Private Helpers
// -------------------------------------------------------------------------

void SmartAquarium::readSensors() {
  static unsigned long lastReadTime = 0;
  
  // Read sensors every 2 seconds (Non-blocking)
  if (millis() - lastReadTime > 2000) {
    lastReadTime = millis();

    // --- SIMULATION DATA (Replace with real sensors) ---
    readings.water_temp = 24.0 + (random(-10, 10) / 10.0); // 23.0 - 25.0
    readings.tds_level = 150 + random(0, 5);
  }
}

void SmartAquarium::applyHardwareState() {
  // 1. Control Bubbler
  digitalWrite(_pumpPin, settings.bubbler_isOn ? HIGH : LOW);

  // 2. Control Lights
  // Logic: If display is OFF, force lights LOW. Otherwise use LED values.
  if (!settings.display_isOn) {
    digitalWrite(_lightPin, LOW); 
  } else {
    // Simple digital simulation for now
    bool anyColor = (settings.r_LED > 0 || settings.g_LED > 0 || settings.b_LED > 0);
    digitalWrite(_lightPin, anyColor ? HIGH : LOW);
  }
}

// -------------------------------------------------------------------------
// Public JSON Wrappers
// -------------------------------------------------------------------------

String SmartAquarium::getSettingsJSON() {
  return settings.serialize();
}

String SmartAquarium::getReadingsJSON() {
  return readings.serialize();
}