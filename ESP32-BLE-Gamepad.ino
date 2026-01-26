#include <Arduino.h>
#include <BleGamepad.h>

#define adcMAX 4095 // ESP32 max input resolution (12-bits)
#define gamepadMAX 32767 // Standard gamepad output (signed 16-bit)
#define pollingInterval 20 // 20ms = 50Hz polling rate
#define buttonCount 2 // Number of buttons, to be updated as buttons are added (L3, R3)
#define debounceDelay 15 // 15ms debounce time

// Stick Pin Definitions
const int PIN_LX = 34;
const int PIN_LY = 35;
const int PIN_RX = 36;
const int PIN_RY = 39;

struct inputMap 
{
  const uint8_t pin; // Associated pin on ESP32
  const uint8_t hidMap; // Button ID sent to PC
  bool isPressed; // Pressed or not pressed
  bool lastReading; // Reading from last loop
  unsigned long lastDebounceTime; // Timer for debounce
};

struct gamepadState
{
  // Analogue stick axes
  int rawLX, rawLY, rawRX, rawRY;
  int outLX, outLY, outRX, outRY;

  // Button array
  inputMap buttons[buttonCount] =
  {
    {32, BUTTON_14, false, false, 0}, // L3
    {33, BUTTON_15, false, false, 0} // R3
  };
};

// Bluetooth device name
gamepadState state;
BleGamepad bleGamepad("ESP32 Gamepad", "dev-exe", 100);

// For serial monitor in Arduino IDE (to be removed)
unsigned long lastLoopTime = 0;

// Function prototypes
void readInputs();
void processInputs();
void sendReport();
void printDebug();

void setup() 
{
  // For serial monitor in Arduino IDE (to be removed)
  Serial.begin(115200);
  Serial.println("Start: ");

  // Hardware setup
  // Enables pullup resistor for BTN_L and R pins, prevents floating
  // Default state is HIGH, button press causes LOW
  for (int i = 0; i < buttonCount; i++)
  {
    pinMode(state.buttons[i].pin, INPUT_PULLUP);
  }

  // Bluetooth HID configuration
  BleGamepadConfiguration bleGamepadConfig;

  //Defines device as a "Gamepad" HID usage ID 0x05, increases compatibility
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepad.begin(&bleGamepadConfig);
}

void loop() 
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastLoopTime >= pollingInterval)
  {
    lastLoopTime = currentMillis;

    if (bleGamepad.isConnected())
    {
      readInputs();
      processInputs();
      sendReport();
      printDebug();
    }
  }
}

void readInputs()
{
  // Read analogue sticks
  state.rawLX = analogRead(PIN_LX);
  state.rawLY = analogRead(PIN_LY);
  state.rawRX = analogRead(PIN_RX);
  state.rawRY = analogRead(PIN_RY);

  // Read buttons
  unsigned long currentMillis = millis();
  
  for (int i = 0; i < buttonCount; i++)
  {
    // Read pin
    bool reading = (digitalRead(state.buttons[i].pin) == LOW);

    // If reading changes timer resets
    if (reading != state.buttons[i].lastReading)
    {
      state.buttons[i].lastDebounceTime = currentMillis;
    }

    // If reading is stable update state
    if ((currentMillis - state.buttons[i].lastDebounceTime) > debounceDelay)
    {
      if (reading != state.buttons[i].isPressed)
      {
        state.buttons[i].isPressed = reading;
      }
    }

    // Save reading for next loop
    state.buttons[i].lastReading = reading;
  }

}

void processInputs()
{
  // Convert ESP32 12-bit input (0-4095) to standard 16-bit Gamepad output (0-32737)
  // Left stick
  state.outLX = map(state.rawLX, 0, adcMAX, 0, gamepadMAX);
  state.outLY = map(state.rawLY, 0, adcMAX, 0, gamepadMAX);
  // Right stick
  state.outRX = map(state.rawRX, 0, adcMAX, 0, gamepadMAX);
  state.outRY = map(state.rawRY, 0, adcMAX, 0, gamepadMAX);
}

void sendReport()
{
  // Communication with PC
  // Send axes
  bleGamepad.setLeftThumb(state.outLX, state.outLY);
  bleGamepad.setRX(state.outRX);
  bleGamepad.setRY(state.outRY);

  // Send buttons
  for (int i = 0; i < buttonCount; i++)
  {
    if (state.buttons[i].isPressed)
    {
      bleGamepad.press(state.buttons[i].hidMap);
    }
    else
    {
      bleGamepad.release(state.buttons[i].hidMap);
    }
  }
}

void printDebug() {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) { // Updates twice a second
        // Left Stick
        Serial.print("L: "); 
        Serial.print(state.outLX); Serial.print(","); 
        Serial.print(state.outLY);
        Serial.print(" [L3:"); Serial.print(state.buttons[0].isPressed); Serial.print("]");

        // Spacer tab
        Serial.print("\t"); 

        // Right Stick
        Serial.print("R: "); 
        Serial.print(state.outRX); Serial.print(","); 
        Serial.print(state.outRY);
        Serial.print(" [R3:"); Serial.print(state.buttons[1].isPressed); Serial.println("]");
        
        lastPrint = millis();
    }
}