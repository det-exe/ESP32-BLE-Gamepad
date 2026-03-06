#ifndef MOTION_H
#define MOTION_H

#include <Arduino.h>

// Define structure for processed motion axis data
struct motionState
{
  // Track mapped roll value
  int16_t outZ;
  // Track mapped pitch value
  int16_t outRZ;
};

// Initialise hardware serial communication for motion sensor
void setupMotion();
// Parse incoming serial data and map logical motion axes
void processMotion(motionState &motion);
// Update active sensor input limit mapping
void setMotionSensitivity(int newSensitivity);

#endif