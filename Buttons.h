#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

const int buttonCount = 6;
// Number of buttons, to be updated as buttons are added
// Action (4) + L3, R3
const int debounceDelay = 15; // 15ms debounce time

struct inputMap
{
  const uint8_t pin; // Associated pin on ESP32
  const uint8_t hidMap; // Button ID sent to PC
  bool isPressed; // Pressed or not pressed
  bool lastReading; // Reading from last loop
  unsigned long lastDebounceTime; // Timer for debounce
};

// Expose the buttons array so the main file can access it for sending reports
extern inputMap buttons[buttonCount];

void setupButtons();
void readButtons();

#endif