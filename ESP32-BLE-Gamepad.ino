#include <Arduino.h>
#include <BleGamepad.h>
#include "AnalogueSticks.h"
#include "Buttons.h"

// Define 125Hz polling rate interval at 8ms
const int pollingInterval = 8;

// Define structure for raw and processed analogue axis data
stickState sticks;

// Initialise BLE gamepad with name, manufacturer and initial battery level
BleGamepad bleGamepad("ESP32 Gamepad", "dev-exe", 100);

// Track previous loop execution time for non-blocking polling
unsigned long lastLoopTime = 0;

void setup()
{
  // Start serial communication for Arduino IDE monitor
  Serial.begin(115200);
  Serial.println("Start: ");

  // Increase ADC range to maximum voltage with 11dB
  analogSetAttenuation(ADC_11db); 

  // Initialise split stick logic
  setupSticks();

  // Initialise hardware button pins
  setupButtons();

  // Configure Bluetooth HID settings
  BleGamepadConfiguration bleGamepadConfig;

  // Set controller type to generic Gamepad for broad compatibility
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepad.begin(&bleGamepadConfig);
}

void loop()
{
  // Check for serial commands
  if (Serial.available())
  {
    char cmd = Serial.read();
    // Trigger automatic calibration with c command
    if (cmd == 'c') calibrateSticks();
    // Update inner deadzone threshold with d command and numeric value
    if (cmd == 'd') setInnerDeadzone(Serial.parseInt());
  }

  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime >= pollingInterval)
  {
    lastLoopTime = currentTime;
    if (bleGamepad.isConnected())
    {
      // Process analogue stick logic
      readSticks(sticks);
      processSticks(sticks);
      printDebug(sticks);

      // Process hardware button logic
      readButtons();

      // Send updated HID report
      bleGamepad.setLeftThumb(sticks.outLX, sticks.outLY);
      bleGamepad.setRX(sticks.outRX);
      bleGamepad.setRY(sticks.outRY);

      for (int i = 0; i < buttonCount; i++)
      {
        if (buttons[i].isPressed)
        {
          bleGamepad.press(buttons[i].hidMap);
        }
        else
        {
          bleGamepad.release(buttons[i].hidMap);
        }
      }
    }
  }
}