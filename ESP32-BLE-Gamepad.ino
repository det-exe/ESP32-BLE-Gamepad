#include <Arduino.h>
#include <BleGamepad.h>
#include "AnalogueSticks.h"

const int debounceDelay = 15; // 15ms debounce time
const int pollingInterval = 8; // 8ms = 125Hz polling rate
const int buttonCount = 2; // Number of buttons, to be updated as buttons are added
// L3, R3

struct inputMap
{
  const uint8_t pin; // Associated pin on ESP32
  const uint8_t hidMap; // Button ID sent to PC
  bool isPressed; // Pressed or not pressed
  bool lastReading; // Reading from last loop
  unsigned long lastDebounceTime; // Timer for debounce
};

// Global instances
stickState sticks; // From AnalogueSticks.h
inputMap buttons[buttonCount] =
{
  {32, BUTTON_14, false, false, 0}, // L3
  {33, BUTTON_15, false, false, 0} // R3
};

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
      // Stick Logic
      readSticks(sticks);
      processSticks(sticks);
      printDebug(sticks);

      // Button Logic
      for (int i = 0; i < buttonCount; i++)
      {
        // Read pin
        bool reading = (digitalRead(buttons[i].pin) == LOW);
        // Reset debounce timer if signal is unstable
        if (reading != buttons[i].lastReading)
        {
          buttons[i].lastDebounceTime = currentTime;
        }

        // Check if signal is stable based on set debounce threshold
        if ((currentTime - buttons[i].lastDebounceTime) > debounceDelay)
        {
          // Update state only if stable reading differs from current state
          if (reading != buttons[i].isPressed)
          {
            buttons[i].isPressed = reading;
          }
        }

        // Save reading for next loop
        buttons[i].lastReading = reading;
      }

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