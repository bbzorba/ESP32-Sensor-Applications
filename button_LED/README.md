
# Button Controlled LED Example

This project demonstrates two modes of controlling an LED with a button on ESP32:

## Hardware Required

- ESP32 development board
- LED connected to GPIO2
- Button connected to GPIO0 (change pin in code if needed)

## Modes

### 1. Continuous Mode (`button_LED.cpp`)

In this mode, the LED is ON while the button is pressed and OFF when released. The state updates every 10ms for responsive control.

**How it works:**
- Press and hold the button: LED turns ON.
- Release the button: LED turns OFF.

### 2. Single Press Toggle Mode (`button_LED_cont.cpp`)

In this mode, each button press toggles the LED state (ON/OFF). The code detects the rising edge (transition from not pressed to pressed) to avoid multiple toggles during a single press.

**How it works:**
- Press the button once: LED toggles (ON if it was OFF, OFF if it was ON).
- Hold the button: LED state does not change until the next press.

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

- Change the LED or button GPIO pins in the source files if needed.
- Adjust the debounce delay (`vTaskDelay`) for your hardware.

## Troubleshooting

- If the LED does not respond, check your wiring and pin assignments.
- Make sure the button is connected with a pull-up resistor or use the internal pull-up as in the code.

## Example Circuit

```
Button (GPIO0) ----||---- GND
                        |
                      [ESP32]

LED (GPIO2) ----->|---- GND
```

## Example Output

- Continuous mode: LED follows button press.
- Single press mode: LED toggles on each press.
