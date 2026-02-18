#include "Buttons.h"
// Include header required for button constants
#include <BleGamepad.h> 

// Define global button array
inputMap buttons[buttonCount] =
{
  // Define action buttons 
  // Define action down button mapping A or Cross
  {18, BUTTON_1, false, false, 0},
  // Define action right button mapping B or Circle
  {19, BUTTON_2, false, false, 0},
  // Define action left button mapping X or Square
  {5, BUTTON_4, false, false, 0},
  // Define action up button mapping Y or Triangle
  {23, BUTTON_5, false, false, 0},

  // Define stick buttons
  // Define left stick click
  {32, BUTTON_14, false, false, 0},
  // Define right stick click
  {32, BUTTON_15, false, false, 0}
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

// Define global directional pad array
// Order elements strictly as up, down, left and right
dpadInput dpad[4] =
{
  // Define up directional pin
  {14, false, false, 0}, 
  // Define down directional pin
  {27, false, false, 0}, 
  // Define left directional pin
  {26, false, false, 0}, 
  // Define right directional pin
  {25, false, false, 0}  
};

void setupDpad()
{
  // Configure hardware pins
  // Enable pullup resistors for all directional pad pins to prevent floating signals
  // Establish default HIGH state where physical press causes LOW signal
  for (int i = 0; i < 4; i++)
  {
    pinMode(dpad[i].pin, INPUT_PULLUP);
  }
}

uint8_t readDpadState()
{
  unsigned long currentTime = millis();

  // Process hardware directional pad states
  for (int i = 0; i < 4; i++)
  {
    // Read physical pin state
    bool reading = (digitalRead(dpad[i].pin) == LOW);

    // Reset debounce timer if signal is unstable
    if (reading != dpad[i].lastReading)
    {
      dpad[i].lastDebounceTime = currentTime;
    }

    // Check if signal is stable based on set debounce threshold
    if ((currentTime - dpad[i].lastDebounceTime) > debounceDelay)
    {
      // Update state only if stable reading differs from current state
      if (reading != dpad[i].isPressed)
      {
        dpad[i].isPressed = reading;
      }
    }
    
    // Save reading for next loop
    dpad[i].lastReading = reading;
  }

  // Map physical button states to logical directional variables
  bool up = dpad[0].isPressed;
  bool down = dpad[1].isPressed;
  bool left = dpad[2].isPressed;
  bool right = dpad[3].isPressed;

  // Return corresponding hat switch integer for active diagonal inputs
  if (up && right) return DPAD_UP_RIGHT;
  if (up && left) return DPAD_UP_LEFT;
  if (down && right) return DPAD_DOWN_RIGHT;
  if (down && left) return DPAD_DOWN_LEFT;
  
  // Return corresponding hat switch integer for active cardinal inputs
  if (up) return DPAD_UP;
  if (down) return DPAD_DOWN;
  if (left) return DPAD_LEFT;
  if (right) return DPAD_RIGHT;

  // Return default centred integer when no inputs are active
  return DPAD_CENTERED;
}