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
const int pinLX = 36;
const int pinLY = 39;
const int pinRX = 34;
const int pinRY = 35;

// Define digital trigger hardware pins
const int pinL2 = 23;
const int pinR2 = 18;

// Define standard signed 16-bit trigger output
const int triggerMax = 32767;

// Define structure for raw and processed analogue axis data
struct stickState
{
  int rawLX, rawLY, rawRX, rawRY;
  int outLX, outLY, outRX, outRY;
};

// Define structure for processed trigger axis data
struct triggerState
{
  int outL2, outR2;
};

// Initialise analogue stick hardware pins and load calibration
void setupSticks();
// Read and average physical analogue stick states
void readSticks(stickState &sticks);
// Process raw values into mapped standard HID coordinates
void processSticks(stickState &sticks);
// Calculate and save new centre point calibration
void calibrateSticks();
// Validate and update inner deadzone threshold
void setInnerDeadzone(int newDZ);
// Output raw and mapped stick values to serial monitor
void printDebug(stickState &sticks);
// Initialise digital trigger hardware pins
void setupTriggers();
// Process physical digital trigger states
void readTriggers(triggerState &triggers);

#endif