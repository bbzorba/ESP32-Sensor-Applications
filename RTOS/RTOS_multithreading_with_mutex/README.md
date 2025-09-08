# RTOS_ex_2: ESP32 FreeRTOS Multithreading Example (C++)

## Overview
This example demonstrates ESP32 FreeRTOS multithreading in C++ using:
- A button and LED (GPIO)
- BME688 gas sensor (I2C)

## Structure
- `main/button_led.hpp, button_led.cpp`: Button and LED classes
- `main/bme688_sensor.hpp, bme688_sensor.cpp`: BME688 sensor class
- `main/app_main.cpp`: FreeRTOS tasks and entry point

## Functionality
- **Button/LED Task:** Reads button state and sets LED accordingly (GPIO0 and GPIO2 by default).
- **BME688 Task:** Reads gas resistance from BME688 every 2 seconds and prints to serial.
- Both tasks run concurrently using FreeRTOS.

## How to Build
1. Place this folder in your ESP-IDF workspace.
2. Run `idf.py build` or use your Makefile as appropriate.
3. Flash to ESP32 and open serial monitor.

## Pinout
- Button: GPIO0 (change in code if needed)
- LED: GPIO2
- BME688: I2C SCL=GPIO22, SDA=GPIO21

## Output Example
```
[BME688] Gas Resistance: 12345.67 Ohms
```
LED will turn on when button is pressed (logic low).

---
