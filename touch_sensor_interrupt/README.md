
# LED Blink Example

This project demonstrates how to blink an LED on an ESP32 using FreeRTOS and C++.

## Hardware Required

- ESP32 development board
- LED connected to GPIO2 (already on the board but change the pin in code if needed)

## How It Works

The LED is turned ON for 500ms and OFF for 500ms in a continuous loop, creating a 1Hz blink rate.

**How it works:**
- LED ON for 0.5 seconds
- LED OFF for 0.5 seconds
- Repeats indefinitely

## How to Build and Flash

1. Connect your ESP32 board and required components.
2. Build the project:
	```bash
	idf.py build
	```
3. Flash to the board:
	```bash
	idf.py -p PORT flash
	```
4. Monitor serial output (optional):
	```bash
	idf.py -p PORT monitor
	```

## Customization

- Change the LED GPIO pin in the source file if needed.
- Adjust the ON/OFF delay for different blink rates.

## Troubleshooting

- If the LED does not blink, check your wiring and pin assignments.
- Make sure the LED is connected with a suitable resistor if required.

## Example Circuit

```
LED (GPIO2) ----->|---- GND
```

## Example Output

- The LED blinks ON and OFF every second.
