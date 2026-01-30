#include <Arduino.h>
#include <BleGamepad.h>
#include "AnalogueSticks.h"
#include "Buttons.h"

const int pollingInterval = 8; // 8ms = 125Hz polling rate
// L3, R3

// Global instances
stickState sticks; // From AnalogueSticks.h

// Initialise BLE gamepad with name, manufacturer and initial battery level
BleGamepad bleGamepad("ESP32 Gamepad", "dev-exe", 100);

// For serial monitor in Arduino IDE (to be removed)
unsigned long lastLoopTime = 0;

void setup()
{
  // For serial monitor in Arduino IDE
  Serial.begin(115200);
  Serial.println("Start: ");
  analogSetAttenuation(ADC_11db);

  // Initialise split stick logic
  setupSticks();
  setupButtons();

  // Hardware setup
  // Enables pullup resistor for BTN_L and R pins, prevents floating
  // Default state is HIGH, button press causes LOW
  for (int i = 0; i < buttonCount; i++)
  {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  // Bluetooth HID configuration
  BleGamepadConfiguration bleGamepadConfig;

  // Set controller type to generic Gamepad to for broad compatibility
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepad.begin(&bleGamepadConfig);
}

void loop()
{
  // Check for serial commands
  if (Serial.available())
  {
    char cmd = Serial.read();
    // 'c' to auto calibrate
    if (cmd == 'c') calibrateSticks();
    // 'd *value*' to manually change deadzone
    if (cmd == 'd') setInnerDeadzone(Serial.parseInt());
  }

  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime >= pollingInterval)
  {
    lastLoopTime = currentTime;
    if (bleGamepad.isConnected())
    {
      // Stick logic
      readSticks(sticks);
      processSticks(sticks);
      printDebug(sticks);

      // Button Logic
      readButtons();

      // Send Report
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