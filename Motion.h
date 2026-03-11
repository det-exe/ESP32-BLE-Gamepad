#ifndef MOTION_H
#define MOTION_H

#include <Arduino.h>

// Define structure for processed motion axis data
struct motionState
{
  // Track mapped roll value
  int16_t throttle;
  // Track mapped pitch value
  int16_t rudder;
};

// Initialise hardware serial communication for motion sensor
void setupMotion();
// Map logical motion axes from incoming serial data
void processMotion(motionState &motion);
// Update active sensor input limit mapping
void setMotionSensitivity(int newSensitivity);

#endif