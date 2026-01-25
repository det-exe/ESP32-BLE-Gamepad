#include <Arduino.h>
#include <bleGamepad.h>

// Bluetooth device name
BleGamepad bleGamepad("ESP32 Gamepad", "dev-exe", 100)

// Pin definitions
// Left analogue stick
const int pinLX = 34;
const int pinLY = 35;
#define BTN_L_PIN 32

// Right analogue stick
const int pinRX = 36;
const int pinRY = 39;
#define BTN_R_PIN 33


// For serial monitor in Arduino IDE (to be removed)
unsigned long lastSerialTime = 0;
const int serialInterval = 500;

void setup() {
  // For serial monitor in Arduino IDE (to be removed)
  Serial.begin(115200)
  Serial.println("Start: ");

  // Hardware setup
  // Enables pullup resistor for BTN_L and R pins, prevents floating
  // Default state is HIGH, button press causes LOW
  pinMode(BTN_L_PIN, INPUT_PULLUP);
  pinMode(BTN_R_PIN, INPUT_PULLUP);

  // Bluetooth HID configuration
  BleGamepadConfiguration bleGamepadConfig;

  //Defines device as a "Gamepad" HID usage ID 0x05, increases compatibility
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepad.begin(&bleGamepadConfig);
}

void loop() {
  if (bleGamepad.isConnected())
  {
    // Read analogue sticks
    int rawLX = analogRead(pinLX);
    int rawLY = analogRead(pinLY);
    int rawRX = analogRead(pinRX);
    int rawRY = analogRead(pinRY);

    // Convert ESP32 12-bit input (0-4095) to standard 16-bit Gamepad output (0-32737)
    // Left stick
    int outLX = map(rawLX, 0, 4095, 0, 32737);
    int outLY = map(rawLY, 0, 4095, 0, 32737);

    // Right stick
    int outRX = map(rawRX, 0, 4095, 0, 32737);
    int outRY = map(rawRY, 0, 4095, 0, 32737);

    // Communication with PC
    bleGamepad.setLeftThumb(outLX, outLY);
    bleGamepad.setRX(outRX);
    bleGamepad.setRY(outRY);

    if (millis() - lastSerialTime > serialInterval)
    {
      Serial.print("L: "); Serial.print(rawLX); Serial.print(","); Serial.print(rawLY);
      Serial.print("\t R: "); Serial.print(rawRX); Serial.print(","); Serial.print(rawRY);
      lastSerialTime = millis();
    }
  }

  delay(20);
}
