#include "AnalogueSticks.h"
#include <math.h>

// Enable persistent non-volatile storage for calibration data
Preferences prefs;

// Define stick axis centre variables for calibration
int centreLX, centreLY, centreRX, centreRY;
int innerDeadzone;

// Define physical limit variables for multi-point calibration
int minLX, maxLX, minLY, maxLY;
int minRX, maxRX, minRY, maxRY;

// Map raw value across split ranges with deadzone limits
int mapSplit(int val, int inMin, int incentre, int inMax, int outMin, int outcentre, int outMax)
{
  if (abs(val - incentre) < innerDeadzone) return outcentre;

  val = constrain(val, inMin + outerDeadzone, inMax - outerDeadzone);

  if (val <= incentre)
  {
    return map(val, inMin + outerDeadzone, incentre, outMin, outcentre);
  }
  else
  {
    return map(val, incentre, inMax - outerDeadzone, outcentre, outMax);
  }
}

// Constrain coordinate output to a precise circular boundary
void constrainToCircle(int *axisX, int *axisY)
{
  int32_t centre = gamepadMax / 2;
  int32_t maxRadius = gamepadMax / 2;

  // Shift coordinates to evaluate distance from origin
  int32_t centredX = *axisX - centre;
  int32_t centredY = *axisY - centre;

  // Calculate exact vector magnitude using floating-point mathematics
  float magnitude = sqrt(pow(centredX, 2) + pow(centredY, 2));

  // Clamp vector to maximum radius boundary
  if (magnitude > maxRadius)
  {
    centredX = (centredX * maxRadius) / magnitude;
    centredY = (centredY * maxRadius) / magnitude;

    // Restore standard coordinate system origin
    *axisX = centredX + centre;
    *axisY = centredY + centre;
  }
}

void setupSticks()
{
  prefs.begin("gamepad", false); 

  centreLX = prefs.getInt("Lx", adcMax / 2);
  centreLY = prefs.getInt("Ly", adcMax / 2);
  centreRX = prefs.getInt("Rx", adcMax / 2);
  centreRY = prefs.getInt("Ry", adcMax / 2);

  minLX = prefs.getInt("minLx", 0);
  maxLX = prefs.getInt("maxLx", adcMax);
  minLY = prefs.getInt("minLy", 0);
  maxLY = prefs.getInt("maxLy", adcMax);

  minRX = prefs.getInt("minRx", 0);
  maxRX = prefs.getInt("maxRx", adcMax);
  minRY = prefs.getInt("minRy", 0);
  maxRY = prefs.getInt("maxRy", adcMax);
  
  innerDeadzone = prefs.getInt("dz", 150);

  Serial.println("Sticks Initialised & Calibration Loaded");
}

void readSticks(stickState &sticks)
{
  long sumLX = 0, sumLY = 0, sumRX = 0, sumRY = 0;
  
  // Define reduced sample count for Hall effect sensors
  int localSampleCount = 5;

  for (int i = 0; i < localSampleCount; i++)
  {
    sumLX += analogRead(pinLX);
    sumLY += analogRead(pinLY);
    sumRX += analogRead(pinRX);
    sumRY += analogRead(pinRY);
  }

  sticks.rawLX = sumLX / localSampleCount;
  sticks.rawLY = sumLY / localSampleCount;
  sticks.rawRX = sumRX / localSampleCount;
  sticks.rawRY = sumRY / localSampleCount;
}

void processSticks(stickState &sticks)
{
  int mid = gamepadMax / 2;

  // Map left stick with inverted X-axis and dynamic limits
  sticks.outLX = mapSplit(sticks.rawLX, minLX, centreLX, maxLX, 0, mid, gamepadMax);
  sticks.outLY = mapSplit(sticks.rawLY, minLY, centreLY, maxLY, 0, mid, gamepadMax);

  // Map right stick with inverted X-axis and dynamic limits
  sticks.outRX = mapSplit(sticks.rawRX, minRX, centreRX, maxRX, 0, mid, gamepadMax);
  sticks.outRY = mapSplit(sticks.rawRY, minRY, centreRY, maxRY, 0, mid, gamepadMax);

  constrainToCircle(&sticks.outLX, &sticks.outLY);
  constrainToCircle(&sticks.outRX, &sticks.outRY);
}

void calibrateSticks()
{
  Serial.println("Leave sticks in resting centre position.");
  
  delay(2000); 

  long tLX = 0, tLY = 0, tRX = 0, tRY = 0;
  int s = 50;

  for (int i = 0; i < s; i++)
  {
    tLX += analogRead(pinLX);
    tLY += analogRead(pinLY);
    tRX += analogRead(pinRX);
    tRY += analogRead(pinRY);
    
    delay(2); 
  }

  centreLX = tLX / s;
  centreLY = tLY / s;
  centreRX = tRX / s;
  centreRY = tRY / s;

  minLX = maxLX = centreLX;
  minLY = maxLY = centreLY;
  minRX = maxRX = centreRX;
  minRY = maxRY = centreRY;

  Serial.println("Rotate both sticks to their maximum physical limits continuously.");

  unsigned long startTime = millis();

  while (millis() - startTime < 10000)
  {
    int curLX = analogRead(pinLX);
    int curLY = analogRead(pinLY);
    int curRX = analogRead(pinRX);
    int curRY = analogRead(pinRY);

    if (curLX < minLX) minLX = curLX;
    if (curLX > maxLX) maxLX = curLX;
    
    if (curLY < minLY) minLY = curLY;
    if (curLY > maxLY) maxLY = curLY;

    if (curRX < minRX) minRX = curRX;
    if (curRX > maxRX) maxRX = curRX;
    
    if (curRY < minRY) minRY = curRY;
    if (curRY > maxRY) maxRY = curRY;

    delay(5);
  }

  prefs.putInt("Lx", centreLX);
  prefs.putInt("Ly", centreLY);
  prefs.putInt("Rx", centreRX);
  prefs.putInt("Ry", centreRY);

  prefs.putInt("minLx", minLX);
  prefs.putInt("maxLx", maxLX);
  prefs.putInt("minLy", minLY);
  prefs.putInt("maxLy", maxLY);

  prefs.putInt("minRx", minRX);
  prefs.putInt("maxRx", maxRX);
  prefs.putInt("minRy", minRY);
  prefs.putInt("maxRy", maxRY);

  Serial.println("Multi-point calibration complete.");
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

void setupTriggers()
{
  pinMode(pinL2, INPUT_PULLUP);
  pinMode(pinR2, INPUT_PULLUP);
}

void readTriggers(triggerState &triggers)
{
  triggers.outL2 = (digitalRead(pinL2) == LOW) ? triggerMax : 0;
  triggers.outR2 = (digitalRead(pinR2) == LOW) ? triggerMax : 0;
}