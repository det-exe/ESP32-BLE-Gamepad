#include "AnalogueSticks.h"

// Enables persistant storage for calibration data
Preferences prefs;

// Stick axis centre for calibration
int centerLX, centerLY, centerRX, centerRY;
int innerDeadzone;

// --- INTERNAL HELPERS ---

int mapSplit(int val, int inMin, int inCenter, int inMax, int outMin, int outCenter, int outMax)
{
  // Inner deadzone, returns center if input within threshold
  if (abs(val - inCenter) < innerDeadzone) return outCenter;

  // Clamps input to outer deadzone limits to prevent overflow
  val = constrain(val, inMin + outerDeadzone, inMax - outerDeadzone);

  // Asymmetric mapping splits range at calibration center
  if (val <= inCenter)
  {
    // Lower half mapping
    return map(val, inMin + outerDeadzone, inCenter, outMin, outCenter);
  }
  else
  {
    // Upper half mapping
    return map(val, inCenter, inMax - outerDeadzone, outCenter, outMax);
  }
}

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

// --- EXPORTED FUNCTIONS ---

void setupSticks()
{
  prefs.begin("gamepad", false); // False meaning storage in R/W mode

  // Load saved values or use defaults
  centerLX = prefs.getInt("Lx", adcMax / 2);
  centerLY = prefs.getInt("Ly", adcMax / 2);
  centerRX = prefs.getInt("Rx", adcMax / 2);
  centerRY = prefs.getInt("Ry", adcMax / 2);
  innerDeadzone = prefs.getInt("dz", 150); // Default 150

  Serial.println("Sticks Initialised & Calibration Loaded");
}

void readSticks(stickState &sticks)
{
  // Read analogue sticks
  // Accumulates samples to average out electrical noise
  long sumLX = 0, sumLY = 0, sumRX = 0, sumRY = 0;

  for (int i = 0; i < sampleCount; i++)
  {
    sumLX += analogRead(pinLX);
    sumLY += analogRead(pinLY);
    sumRX += analogRead(pinRX);
    sumRY += analogRead(pinRY);
  }

  // Calculates average reading
  sticks.rawLX = sumLX / sampleCount;
  sticks.rawLY = sumLY / sampleCount;
  sticks.rawRX = sumRX / sampleCount;
  sticks.rawRY = sumRY / sampleCount;
}

void processSticks(stickState &sticks)
{
  // Convert ESP32 12-bit input (0-4095) to standard 16-bit Gamepad output (0-32767)
  int mid = gamepadMax / 2;

  // Left Stick
  sticks.outLX = mapSplit(sticks.rawLX, 0, centerLX, adcMax, gamepadMax, mid, 0);
  sticks.outLY = mapSplit(sticks.rawLY, 0, centerLY, adcMax, 0, mid, gamepadMax);

  // Right Stick
  sticks.outRX = mapSplit(sticks.rawRX, 0, centerRX, adcMax, gamepadMax, mid, 0);
  sticks.outRY = mapSplit(sticks.rawRY, 0, centerRY, adcMax, 0, mid, gamepadMax);

  // Apply circularisation
  constrainToCircle(&sticks.outLX, &sticks.outLY);
  constrainToCircle(&sticks.outRX, &sticks.outRY);
}

void calibrateSticks()
{
  Serial.println("Calibrating.");
  delay(1000); // Give user time to let go of sticks

  long tLX = 0, tLY = 0, tRX = 0, tRY = 0;
  int s = 50;

  // Take readings and sum them up
  for (int i = 0; i < s; i++)
  {
    tLX += analogRead(pinLX);
    tLY += analogRead(pinLY);
    tRX += analogRead(pinRX);
    tRY += analogRead(pinRY);
    delay(2);
    // Small delay to ensure distinct readings
  }

  // Calculate average (sum / samples)
  centerLX = tLX / s;
  centerLY = tLY / s;
  centerRX = tRX / s;
  centerRY = tRY / s;

  // Save to Permanent Memory
  prefs.putInt("Lx", centerLX);
  prefs.putInt("Ly", centerLY);
  prefs.putInt("Rx", centerRX);
  prefs.putInt("Ry", centerRY);

  Serial.println("Calibration complete.");
}

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
    // Debug: Show RAW input vs MAPPED output
    Serial.print("LX [Raw:");
    Serial.print(sticks.rawLX); // Watch this number!
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