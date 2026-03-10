#include "Motion.h"

// Define GY-25 sensor receive pin
const int gy25RxPin = 17;
// Define GY-25 sensor transmit pin
const int gy25TxPin = 16;

// Allocate array to store incoming sensor bytes
uint8_t serialBuffer[8];
// Track current position within data packet
uint8_t bufferIndex = 0;

// Set default sensitivity level
int currentSensitivity = 5;
// Set default raw sensor input limit
int16_t currentLimit = 9000;

// Map sensitivity levels from zero to ten to raw sensor input limits
const int16_t sensitivityLimits[11] = {18000, 16200, 14400, 12600, 10800, 9000, 7400, 5800, 4200, 2600, 1000};

// Store latest valid mapping calculations globally to decouple parsing from transmission
int32_t currentMappedPitch = 16383;
int32_t currentMappedRoll = 16383;

void setupMotion()
{
  // Start secondary hardware serial connection for sensor
  Serial2.begin(115200, SERIAL_8N1, gy25RxPin, gy25TxPin);

  // Pause execution to allow sensor to boot
  delay(3000);

  // Instruct sensor to begin transmitting automatic updates
  Serial2.write(0xA5);
  Serial2.write(0x52);
}

void processMotion(motionState &motion)
{
  // Drain hardware serial buffer completely as fast as possible
  while (Serial2.available())
  {
    // Read single byte from hardware serial buffer
    uint8_t data = Serial2.read();

    // Ignore extraneous bytes outside valid packet header
    if (bufferIndex == 0 && data != 0xAA)
    {
      continue;
    }

    // Store valid byte in buffer array
    serialBuffer[bufferIndex] = data;

    // Increment index counter for next byte
    bufferIndex++;

    // Evaluate data once buffer reaches eight bytes
    if (bufferIndex == 8)
    {
      // Reset index counter to begin new packet
      bufferIndex = 0;

      // Verify final byte matches expected packet footer
      if (serialBuffer[7] == 0x55)
      {
        // Combine two bytes to form complete raw pitch integer
        int16_t rawPitch = (serialBuffer[3] << 8) | serialBuffer[4];
        // Combine two bytes to form complete raw roll integer
        int16_t rawRoll = (serialBuffer[5] << 8) | serialBuffer[6];

        // Enforce deadzone on raw pitch data
        if (abs(rawPitch) < 500)
        {
          rawPitch = 0;
        }
        // Enforce deadzone on raw roll data
        if (abs(rawRoll) < 500)
        {
          rawRoll = 0;
        }

        // Map raw input to logical output using 32-bit integers to prevent rollover
        int32_t tempPitch = map(rawPitch, -currentLimit, currentLimit, 0, 32767);
        int32_t tempRoll = map(rawRoll, -currentLimit, currentLimit, 0, 32767);

        // Update global variables with new constrained values
        currentMappedPitch = constrain(tempPitch, 0, 32767);
        currentMappedRoll = constrain(tempRoll, 0, 32767);
      }
    }
  }

  // Expose latest valid output to main loop
  // Assign mapped roll to slider 1
  motion.slider1 = currentMappedRoll;
  // Assign mapped pitch to slider 2
  motion.slider2 = currentMappedPitch;
}

void setMotionSensitivity(int newSensitivity)
{
  // Restrict new sensitivity value strictly to allowed range
  newSensitivity = constrain(newSensitivity, 0, 10);

  // Update active sensitivity variable
  currentSensitivity = newSensitivity;

  // Update active input limit variable
  currentLimit = sensitivityLimits[currentSensitivity];

  // Print confirmation message to serial monitor
  Serial.print("Sensitivity updated to: ");
  Serial.println(currentSensitivity);
}