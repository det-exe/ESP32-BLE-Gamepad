#include "Buttons.h"
#include <BleGamepad.h> // Required for BUTTON_14 / BUTTON_15 constants

// Global button array definition
inputMap buttons[buttonCount] =
{
  {32, BUTTON_14, false, false, 0}, // L3
  {33, BUTTON_15, false, false, 0} // R3
};

void setupButtons()
{
  // Hardware setup
  // Enables pullup resistor for BTN_L and R pins, prevents floating
  // Default state is HIGH, button press causes LOW
  for (int i = 0; i < buttonCount; i++)
  {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void readButtons()
{
  unsigned long currentTime = millis();

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
}