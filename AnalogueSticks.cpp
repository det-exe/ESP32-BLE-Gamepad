#include "AnalogueSticks.h"

// Enable persistent non-volatile storage for calibration data
Preferences prefs;

// Define stick axis centre variables for calibration
int centerLX, centerLY, centerRX, centerRY;
int innerDeadzone;

// Map raw value across split ranges with deadzone limits
int mapSplit(int val, int inMin, int inCenter, int inMax, int outMin, int outCenter, int outMax)
{
  // Return centre coordinate if input falls within inner deadzone threshold
  if (abs(val - inCenter) < innerDeadzone) return outCenter;

  // Clamp input to outer deadzone limits to prevent overflow
  val = constrain(val, inMin + outerDeadzone, inMax - outerDeadzone);

  // Apply asymmetric mapping split at calibration centre
  if (val <= inCenter)
  {
    // Map lower half range
    return map(val, inMin + outerDeadzone, inCenter, outMin, outCenter);
  }
  else
  {
    // Map upper half range
    return map(val, inCenter, inMax - outerDeadzone, outCenter, outMax);
  }
}

// Constrain coordinate output to circular boundary
void constrainToCircle(int *axisX, int *axisY)
{
  // Define centre and radius based on max output
  float center = gamepadMax / 2.0;
  float maxRadius = gamepadMax / 2.0;

  // Shift coordinates to centre origin
  float centeredX = *axisX - center;
  float centeredY = *axisY - center;

  // Calculate vector magnitude
  float magnitude = hypot(centeredX, centeredY);

  // Clamp vector to max radius if outside bounds
  if (magnitude > maxRadius)
  {
    // Calculate scaling factor to shrink vector
    float scale = maxRadius / magnitude;
    
    // Apply scaling to X and Y axes
    centeredX *= scale;
    centeredY *= scale;

    // Restore original coordinate system
    *axisX = (int)(centeredX + center);
    *axisY = (int)(centeredY + center);
  }
}

void setupSticks()
{
  // Initialise preferences in read and write mode
  prefs.begin("gamepad", false); 

  // Load saved values or apply defaults
  centerLX = prefs.getInt("Lx", adcMax / 2);
  centerLY = prefs.getInt("Ly", adcMax / 2);
  centerRX = prefs.getInt("Rx", adcMax / 2);
  centerRY = prefs.getInt("Ry", adcMax / 2);
  
  // Apply default inner deadzone threshold
  innerDeadzone = prefs.getInt("dz", 150);

  Serial.println("Sticks Initialised & Calibration Loaded");
}

void readSticks(stickState &sticks)
{
  // Read analogue sticks
  // Accumulate samples to average out electrical noise
  long sumLX = 0, sumLY = 0, sumRX = 0, sumRY = 0;

  for (int i = 0; i < sampleCount; i++)
  {
    sumLX += analogRead(pinLX);
    sumLY += analogRead(pinLY);
    sumRX += analogRead(pinRX);
    sumRY += analogRead(pinRY);
  }

  // Calculate average reading
  sticks.rawLX = sumLX / sampleCount;
  sticks.rawLY = sumLY / sampleCount;
  sticks.rawRX = sumRX / sampleCount;
  sticks.rawRY = sumRY / sampleCount;
}

void processSticks(stickState &sticks)
{
  // Calculate midpoint baseline for standard gamepad output
  int mid = gamepadMax / 2;

  // Invert Y axis mapping to comply with standard HID gamepad orientation
  // Map left stick
  sticks.outLX = mapSplit(sticks.rawLX, 0, centerLX, adcMax, gamepadMax, mid, 0);
  sticks.outLY = mapSplit(sticks.rawLY, 0, centerLY, adcMax, 0, mid, gamepadMax);

  // Map right stick
  sticks.outRX = mapSplit(sticks.rawRX, 0, centerRX, adcMax, gamepadMax, mid, 0);
  sticks.outRY = mapSplit(sticks.rawRY, 0, centerRY, adcMax, 0, mid, gamepadMax);

  // Apply circularisation
  constrainToCircle(&sticks.outLX, &sticks.outLY);
  constrainToCircle(&sticks.outRX, &sticks.outRY);
}

void calibrateSticks()
{
  Serial.println("Calibrating.");
  // Pause execution to allow physical stick release
  delay(1000); 

  long tLX = 0, tLY = 0, tRX = 0, tRY = 0;
  // Define sample count for calibration average
  int s = 50;

  // Accumulate sample readings
  for (int i = 0; i < s; i++)
  {
    tLX += analogRead(pinLX);
    tLY += analogRead(pinLY);
    tRX += analogRead(pinRX);
    tRY += analogRead(pinRY);
    // Pause briefly to ensure distinct readings
    delay(2); 
  }

// Calculate average reading across calibration sample count
  centerLX = tLX / s;
  centerLY = tLY / s;
  centerRX = tRX / s;
  centerRY = tRY / s;

  // Save calculated centres to persistent memory
  prefs.putInt("Lx", centerLX);
  prefs.putInt("Ly", centerLY);
  prefs.putInt("Rx", centerRX);
  prefs.putInt("Ry", centerRY);

  Serial.println("Calibration complete.");
}

// Validate and update inner deadzone threshold
void setInnerDeadzone(int newDZ)
{
  if (newDZ >= 0)
  {
    innerDeadzone = newDZ;
    prefs.putInt("dz", innerDeadzone);
    Serial.print("Inner Deadzone Updated: "); Serial.println(innerDeadzone);
  }
}

void printDebug(stickState &sticks)
{
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 300)
  {
    // Show raw input versus mapped output
    Serial.print("LX [Raw:");
    Serial.print(sticks.rawLX);
    Serial.print(" -> Map:");
    Serial.print(sticks.outLX);
    Serial.print("]");

    Serial.print("  |  LY [Raw:");
    Serial.print(sticks.rawLY);
    Serial.print(" -> Map:");
    Serial.print(sticks.outLY);
    Serial.println("]");

    lastPrint = millis();
  }
}