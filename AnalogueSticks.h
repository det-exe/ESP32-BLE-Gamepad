// Prevent double referencing with include guard
#ifndef ANALOGUE_STICKS_H  
#define ANALOGUE_STICKS_H

#include <Arduino.h>
#include <Preferences.h>

// Define constants
// Define maximum ESP32 12-bit ADC resolution
const int adcMax = 4095;

// Define standard signed 16-bit gamepad output
const int gamepadMax = 32767;

// Define mathematical centre of 16-bit output
const int centerVal = gamepadMax / 2;

// Define hardware limit outer deadzone
const int outerDeadzone = 50;

// Define sample count for electrical noise averaging
const int sampleCount = 50; 

// Define analogue stick hardware pins
const int pinLX = 32;
const int pinLY = 32;
const int pinRX = 32;
const int pinRY = 32;

// Define structure for raw and processed analogue axis data
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