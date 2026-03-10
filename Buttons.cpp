#include "Buttons.h"
// Include header required for button constants
#include <BleGamepad.h> 

// Instantiate global I2C expansion object
Adafruit_MCP23X17 mcp;

// Define hardware interrupt pin for expansion chip
const byte interruptPin = 15;
// Track active interrupt state for debounce processing
volatile bool mcpInterruptTriggered = false;

// Flag interrupt detection for polling loop
void IRAM_ATTR handleMcpInterrupt()
{
  mcpInterruptTriggered = true;
}

// Define global button array
inputMap buttons[buttonCount] =
{
  // Define action buttons 
  // Define action down button mapping
  {5, BUTTON_1, false, false, 0},
  // Define action right button mapping
  {4, BUTTON_2, false, false, 0},
  // Define action left button mapping
  {2, BUTTON_4, false, false, 0},
  // Define action up button mapping
  {13, BUTTON_5, false, false, 0},

  // Define stick buttons
  // Define left stick click
  {27, BUTTON_14, false, false, 0},
// Define right stick click
  {14, BUTTON_15, false, false, 0},

  // Define primary left shoulder mapping
  {22, BUTTON_7, false, false, 0},
  // Define primary right shoulder mapping
  {19, BUTTON_8, false, false, 0}
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

    // Reset debounce timer on unstable signal detection
    if (reading != buttons[i].lastReading)
    {
      buttons[i].lastDebounceTime = currentTime;
    }

    // Evaluate signal stability against debounce threshold
    if ((currentTime - buttons[i].lastDebounceTime) > debounceDelay)
    {
      // Update internal state on confirmed stable change
      if (reading != buttons[i].isPressed)
      {
        buttons[i].isPressed = reading;
      }
    }

    // Store current reading for subsequent loop comparison
    buttons[i].lastReading = reading;
  }
}

// Define global directional pad array
// Order elements strictly as up, down, left and right
dpadInput dpad[4] =
{
  // Define up directional pin on MCP23017 Port B
  {8, false, false, 0}, 
  // Define down directional pin on MCP23017 Port B
  {9, false, false, 0}, 
  // Define left directional pin on MCP23017 Port B
  {10, false, false, 0}, 
  // Define right directional pin on MCP23017 Port B
  {11, false, false, 0}  
};

// Define global expansion button array
mcpInputMap mcpButtons[mcpButtonCount] =
{
  // Define Select button on MCP23017 Port B
  {12, BUTTON_11, false},
  // Define Start button on MCP23017 Port B
  {13, BUTTON_12, false},
  // Define Guide button on MCP23017 Port B
  {14, BUTTON_13, false}
};

void setupDpad()
{
  // Initialise isolated I2C bus for expansion chip
  Wire.begin(25, 26);

  // Initialise communication with expansion chip
  if (!mcp.begin_I2C(0x20, &Wire))
  {
    Serial.println("Error initialising MCP23017.");
    while (1);
  }
  
  // Configure hardware interrupt pin on ESP32
  pinMode(interruptPin, INPUT_PULLUP);
  // Attach hardware interrupt to processing function
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleMcpInterrupt, FALLING);
  
  // Configure expansion chip to mirror interrupts
  // Set interrupt output pin to active low state
  mcp.setupInterrupts(true, false, LOW);

  // Configure directional pad hardware pins
  for (int i = 0; i < 4; i++)
  {
    mcp.pinMode(dpad[i].pin, INPUT_PULLUP);
    mcp.setupInterruptPin(dpad[i].pin, CHANGE);
  }

  // Configure supplementary expansion button hardware pins
  for (int i = 0; i < mcpButtonCount; i++)
  {
    mcp.pinMode(mcpButtons[i].pin, INPUT_PULLUP);
    mcp.setupInterruptPin(mcpButtons[i].pin, CHANGE);
  }
  
  // Clear pending interrupts on startup
  mcp.clearInterrupts();
}

uint8_t readDpadState()
{
  static unsigned long debounceTimer = 0;
  static bool isDebouncing = false;

  // Start debounce timer on interrupt detection
  if ((mcpInterruptTriggered || digitalRead(interruptPin) == LOW) && !isDebouncing)
  {
    isDebouncing = true;
    debounceTimer = millis();
  }

  // Verify completion of debounce interval
  if (isDebouncing && (millis() - debounceTimer > debounceDelay))
  {
    // Process hardware directional pad states
    for (int i = 0; i < 4; i++)
    {
      // Read physical expansion pin state
      dpad[i].isPressed = (mcp.digitalRead(dpad[i].pin) == LOW);
    }

    // Process supplementary expansion button states
    for (int i = 0; i < mcpButtonCount; i++)
    {
      // Read physical expansion pin state
      mcpButtons[i].isPressed = (mcp.digitalRead(mcpButtons[i].pin) == LOW);
    }
    
    // Reset software interrupt flag
    mcpInterruptTriggered = false;
    // Reset debounce active state
    isDebouncing = false;
    // Clear handled expansion chip hardware interrupts
    mcp.clearInterrupts();
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