#include <Arduino.h>
#include <BleGamepad.h>
#include <Preferences.h>

const int adcMAX = 4095; // ESP32 max input resolution (12-bits)
const int gamepadMAX = 32767; // Standard gamepad output (signed 16-bit)
const int pollingInterval = 20; // 20ms = 50Hz polling rate
const int buttonCount = 2; // Number of buttons, to be updated as buttons are added (L3, R3)
const int debounceDelay = 15; // 15ms debounce time
const int outerDeadzone = 75; // Clamps to max before physical limit

// Stick pin definitions
const int PIN_LX = 35;
const int PIN_LY = 34;
const int PIN_RX = 39;
const int PIN_RY = 36;

// Stick axis centre for calibration (default value = 4095/2 = 2048)
int centerLX = 2048;
int centerLY = 2048;
int centerRX = 2048;
int centerRY = 2048;

int innerDeadzone;

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

// Global instance to store current gamepad state
gamepadState state;
// Initialise BLE gamepad with name, manufacturer and initial battery level
BleGamepad bleGamepad("ESP32 Gamepad", "dev-exe", 100);
// Enables persistant storage for calibration data
Preferences prefs;

// For serial monitor in Arduino IDE (to be removed)
unsigned long lastLoopTime = 0;

// Function prototypes
void readInputs();
void processInputs();
void sendReport();
void printDebug();

void loadPreferences() 
{
  prefs.begin("gamepad", false); // False meaning storage in R/W mode
  
  // Load saved values or use defaults
  centerLX = prefs.getInt("Lx", 2048);
  centerLY = prefs.getInt("Ly", 2048);
  centerRX = prefs.getInt("Rx", 2048);
  centerRY = prefs.getInt("Ry", 2048);
  innerDeadzone = prefs.getInt("dz", 150);
  
  Serial.println("Calibration Loaded");
}

void setup() 
{
  // For serial monitor in Arduino IDE
  Serial.begin(115200);
  Serial.println("Start: ");

  loadPreferences();

  // Hardware setup
  // Enables pullup resistor for BTN_L and R pins, prevents floating
  // Default state is HIGH, button press causes LOW
  for (int i = 0; i < buttonCount; i++)
  {
    pinMode(state.buttons[i].pin, INPUT_PULLUP);
  }

  // Bluetooth HID configuration
  BleGamepadConfiguration bleGamepadConfig;

  // Set controller type to generic Gamepad to for broad compatibility
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepad.begin(&bleGamepadConfig);
}

void loop() 
{
  checkSerialCommands(); // UART communication

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
  // Accumulates samples to average out electrical noise
  long sumLX = 0, sumLY = 0, sumRX = 0, sumRY = 0;
  const int samples = 20; // Sample count for smoothing

  for (int i = 0; i < samples; i++) 
  {
      sumLX += analogRead(PIN_LX);
      sumLY += analogRead(PIN_LY);
      sumRX += analogRead(PIN_RX);
      sumRY += analogRead(PIN_RY);
  }

  // Calculates average reading
  state.rawLX = sumLX / samples;
  state.rawLY = sumLY / samples;
  state.rawRX = sumRX / samples;
  state.rawRY = sumRY / samples;

  // Read buttons
  unsigned long currentMillis = millis();
  for (int i = 0; i < buttonCount; i++)
  {
    // Read pin
    bool reading = (digitalRead(state.buttons[i].pin) == LOW);

    // Reset debounce timer if signal is unstable
    if (reading != state.buttons[i].lastReading)
    {
      state.buttons[i].lastDebounceTime = currentMillis;
    }

    // Check if signal is stable based on set debounce threshold
    if ((currentMillis - state.buttons[i].lastDebounceTime) > debounceDelay)
    {
      // Update state only if stable reading differs from current state
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

  int mid = 16383;

  // Left Stick
  state.outLX = mapSplit(state.rawLX, 0, centerLX, adcMAX, gamepadMAX, mid, 0);
  state.outLY = mapSplit(state.rawLY, 0, centerLY, adcMAX, 0, mid, gamepadMAX);

  // Right Stick
  state.outRX = mapSplit(state.rawRX, 0, centerRX, adcMAX, gamepadMAX, mid, 0);
  state.outRY = mapSplit(state.rawRY, 0, centerRY, adcMAX, 0, mid, gamepadMAX);
}

void calibrateCenters() 
{
    Serial.println("Calibrating...");
    delay(1000); // Give user time to let go of sticks
    
    long tLX = 0, tLY = 0, tRX = 0, tRY = 0;
    int s = 100; // Number of samples to take

    // Take readings and sum them up
    for(int i=0; i<s; i++) 
    {
        tLX += analogRead(PIN_LX);
        tLY += analogRead(PIN_LY);
        tRX += analogRead(PIN_RX);
        tRY += analogRead(PIN_RY);
        delay(2); // Small delay to ensure distinct readings
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

// Applies deadzones and mapping to raw input
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

void checkSerialCommands() 
{
  if (Serial.available()) 
  {
    char cmd = Serial.read();

    // 'c' to auto calibrate
    if (cmd == 'c') calibrateCenters();

    // 'd' -> manually change deadzone
    if (cmd == 'd') 
    {
      int newDZ = Serial.parseInt();
      if (newDZ > 0) 
      {
        innerDeadzone = newDZ;
        prefs.putInt("dz", innerDeadzone);
        Serial.print("Inner Deadzone Updated: "); Serial.println(innerDeadzone);
      }
    }
  }
}

void sendReport()
{
  // Communication with PC
  // Update characteristics and notify host

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