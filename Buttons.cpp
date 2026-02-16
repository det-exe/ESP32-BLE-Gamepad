#include "Buttons.h"
// Include header required for button constants
#include <BleGamepad.h> 

// Define global button array
inputMap buttons[buttonCount] =
{
  // Define action buttons 
  // Define action down button mapping A or Cross
  {16, BUTTON_1, false, false, 0},
  // Define action right button mapping B or Circle
  {22, BUTTON_2, false, false, 0},
  // Define action left button mapping X or Square
  {17, BUTTON_4, false, false, 0},
  // Define action up button mapping Y or Triangle
  {18, BUTTON_5, false, false, 0},

  // Define stick buttons
  // Define left stick click
  {23, BUTTON_14, false, false, 0},
  // Define right stick click
  {19, BUTTON_15, false, false, 0}
};

void setupButtons()
{
  // Configure hardware pins
  // Enable pullup resistors for all button pins to prevent floating signals
  // Establish default HIGH state where physical press causes LOW signal
  for (int i = 0; i < buttonCount; i++)
  {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void readButtons()
{
  unsigned long currentTime = millis();

  // Process hardware button states
  for (int i = 0; i < buttonCount; i++)
  {
    // Read physical pin state
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