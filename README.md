# Bluetooth Low Energy Gamepad

This project implements a wireless HID gamepad. It uses an ESP32 microcontroller. It uses the [ESP32 BLE Gamepad Library by LemmingDev](https://github.com/lemmingDev/ESP32-BLE-Gamepad). The system integrates the following features:

* Dual analogue sticks.
* A directional pad.
* Action buttons.
* Motion sensing.

## Analogue Sticks

* Read X and Y axis data for two sticks.
* Process L3 and R3 button inputs.
* Calibrate centre points via serial commands.
* Store calibration data persistently.
* Clamp output vectors to a circular boundary.
* Smooth electrical noise through sample averaging.
* Map 12-bit ADC values to 16-bit limits.
* Configure inner deadzones via serial commands.

## Buttons

* Support four primary action buttons.
* Process directional pad inputs via an MCP23017 I2C expander.
* Utilise hardware interrupts for the directional pad.
* Apply software debouncing to stabilise inputs.

## Motion Sensing

* Integrate a GY-25 sensor for tilt input.
* Map roll data to Slider 1.
* Map pitch data to Slider 2.
* Adjust sensor sensitivity levels through serial commands.

## Digital Triggers

* Map the L2 trigger to the Z axis.
* Map the R2 trigger to the Rz axis.

## Technical Specifications

* Poll inputs at 125Hz.
* Output 16-bit logical axes.
* Connect via Bluetooth Low Energy HID.

## Future Development

* Implement cubic response curves for analogue sticks.
* Develop a graphical configuration interface.
* Utilise Qt and JSON for the interface.
