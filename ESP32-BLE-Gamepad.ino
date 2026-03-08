#include <Arduino.h>
#include <BleGamepad.h>
#include <Wire.h>
#include "AnalogueSticks.h"
#include "Buttons.h"
#include "Motion.h"

// Define 125Hz polling rate interval at 8ms
const int pollingInterval = 8;
// Define structure for raw and processed analogue axis data
stickState sticks;
// Define structure for processed motion axis data
motionState motion;
// Initialise BLE gamepad with name, manufacturer and initial battery level
BleGamepad bleGamepad("ESP32 Gamepad", "dev-exe", 100);
// Track previous loop execution time for non-blocking polling
unsigned long lastLoopTime = 0;

void setup()
{
  // Start serial communication for Arduino IDE monitor
  Serial.begin(115200);
  Serial.println("Start: ");
  // Increase ADC range to maximum voltage with 11dB attenuation
  analogSetAttenuation(ADC_11db);

  // Initialise split stick logic
  setupSticks();
  // Initialise hardware button pins
  setupButtons();
  // Initialise directional pad hardware pins
  setupDpad();
  // Initialise motion sensor hardware serial
  setupMotion();

  // Configure Bluetooth HID settings
  BleGamepadConfiguration bleGamepadConfig;
  // Set controller type to generic Gamepad for broad compatibility
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  // Enforce strictly positive logical limits for the mapping functions
  bleGamepadConfig.setAxesMin(0);
  bleGamepadConfig.setAxesMax(32767);
  // Enable X, Y, Rx, Ry, Z and Rz axes for gamepad output
  bleGamepadConfig.setWhichAxes(true, true, false, true, true, false, true, true);
  // Disable automatic reporting to prevent Bluetooth queue saturation
  bleGamepadConfig.setAutoReport(false);

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
    // Update motion sensitivity with s command and numeric value
    if (cmd == 's') setMotionSensitivity(Serial.parseInt());
  }

  // Process incoming motion data continuously outside the polling block to prevent serial buffer overflow
  processMotion(motion);

  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime >= pollingInterval)
  {
    lastLoopTime = currentTime;

    if (bleGamepad.isConnected())
    {
      // Process analogue stick logic
      readSticks(sticks);
      processSticks(sticks);

      // Process hardware button logic
      readButtons();

      // Assign updated left stick states
      bleGamepad.setLeftThumb(sticks.outLX, sticks.outLY);
      // Assign updated right stick states
      bleGamepad.setRX(sticks.outRX);
      bleGamepad.setRY(sticks.outRY);
      // Assign updated motion states
      bleGamepad.setSlider1(motion.slider1);
      bleGamepad.setSlider2(motion.slider2);

      // Process primary action buttons
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

      // Read physical directional pad state and supplementary buttons
      uint8_t dpadState = readDpadState();

      // Process supplementary expansion buttons
      for (int i = 0; i < mcpButtonCount; i++)
      {
        if (mcpButtons[i].isPressed)
        {
          bleGamepad.press(mcpButtons[i].hidMap);
        }
        else
        {
          bleGamepad.release(mcpButtons[i].hidMap);
        }
      }

      // Send updated directional pad state to hat switch one
      bleGamepad.setHat1(dpadState);

      // Transmit the complete gamepad packet to the operating system
      bleGamepad.sendReport();
    }
  }
}