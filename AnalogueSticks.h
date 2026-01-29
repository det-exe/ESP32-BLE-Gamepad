#ifndef ANALOGUE_STICKS_H
#define ANALOGUE_STICKS_H

#include <Arduino.h>
#include <Preferences.h>

// Constants
const int adcMax = 4095; // ESP32 max input resolution (12-bits)
const int gamepadMax = 32767; // Standard gamepad output (signed 16-bit)
const int centerVal = gamepadMax / 2;
const int outerDeadzone = 50; 
const int sampleCount = 50; 

// Stick pin definitions
const int pinLX = 35;
const int pinLY = 34;
const int pinRX = 39;
const int pinRY = 36;

struct stickState
{
  int rawLX, rawLY, rawRX, rawRY;
  int outLX, outLY, outRX, outRY;
};

void setupSticks();
void readSticks(stickState &sticks);
void processSticks(stickState &sticks);
void calibrateSticks();
void setInnerDeadzone(int newDZ);
void printDebug(stickState &sticks);

#endif