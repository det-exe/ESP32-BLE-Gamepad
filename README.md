# Bluetooth Low Energy Gamepad

This project implements a wireless HID gamepad on an ESP32 microcontroller using the [ESP32 BLE Gamepad library by lemmingDev](https://github.com/lemmingDev/ESP32-BLE-Gamepad). The system integrates the following features:

* Dual analogue sticks
* A directional pad
* Action, shoulder, trigger and stick-click buttons
* Motion sensing

## Analogue Sticks

* Read X and Y axis data for two sticks
* Calibrate centre points and physical limits via serial commands
* Store calibration data persistently in non-volatile storage
* Clamp output vectors to a circular boundary
* Smooth electrical noise through sample averaging
* Map 12-bit ADC values to a 0 to 32767 logical output range
* Configure inner deadzone via serial command

## Buttons

* Eight directly wired buttons covering the four action buttons, two shoulder buttons and two stick clicks
* Directional pad and three system buttons (Start, Select, Guide) routed via an MCP23017 I2C expander
* Hardware interrupts on the expander
* Software debouncing on both the direct and expander paths

## Motion Sensing

* GY-25 sensor for tilt input
* Roll mapped to the throttle simulation axis
* Pitch mapped to the rudder simulation axis
* Sensitivity adjustable through serial commands

## Digital Triggers

* L2 trigger mapped to the Z axis
* R2 trigger mapped to the Rz axis

## Technical Specifications

* 125 Hz input polling rate
* Logical axes ranging 0 to 32767
* Connectivity via Bluetooth Low Energy HID

## Licence

This project is licensed under the MIT License. See `LICENSE` for details.

## Third-party libraries

This project uses the ESP32 BLE Gamepad library by lemmingDev, licensed under the MIT License.
