#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

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
// Enable external access for HID report generation
extern inputMap buttons[buttonCount];

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
// Enable external access for HID report generation
extern dpadInput dpad[4];

void setupDpad();
uint8_t readDpadState();

#endif