#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

// Define total hardware button count
const int buttonCount = 6;

// Define signal debounce duration in milliseconds
const int debounceDelay = 10;

// Define structure for hardware pin and HID report mapping
struct inputMap
{
  // Define associated ESP32 hardware pin
  const uint8_t pin; 
  // Define HID button ID mapped to PC
  const uint8_t hidMap;
  // Track current active press state
  bool isPressed;
  // Track physical reading from previous loop
  bool lastReading;
  // Track timestamp for debounce calculation
  unsigned long lastDebounceTime;
};

// Expose global button array
extern inputMap buttons[buttonCount];
// Expose global I2C expansion object
extern Adafruit_MCP23X17 mcp;
// Expose interrupt flag
extern volatile bool mcpInterruptTriggered;

void setupButtons();
void readButtons();

// Define structure for hardware pin and directional pad state mapping
struct dpadInput
{
  // Define associated ESP32 hardware pin
  const uint8_t pin; 
  // Track current active press state
  bool isPressed; 
  // Track physical reading from previous loop
  bool lastReading; 
  // Track timestamp for debounce calculation
  unsigned long lastDebounceTime; 
};

// Expose global directional pad array
extern dpadInput dpad[4];

// Define structure for supplementary expansion button mapping
struct mcpInputMap
{
  // Define associated MCP23017 hardware pin
  const uint8_t pin;
  // Define HID button ID mapped to PC
  const uint8_t hidMap;
  // Track current active press state
  bool isPressed;
};

// Define total supplementary expansion button count
const int mcpButtonCount = 3;
// Expose global expansion button array
extern mcpInputMap mcpButtons[mcpButtonCount];

void setupDpad();
uint8_t readDpadState();
void IRAM_ATTR handleMcpInterrupt();

#endif