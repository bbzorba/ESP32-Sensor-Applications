
# BME688 Sensor Example & Gas Density Classifier

This project demonstrates how to use the Bosch BME688 sensor with ESP32 to measure temperature, humidity, pressure, and gas resistance, and classify gas types using simple AI logic. It includes:

- **Continuous sensor reading** (see `main/read_bme688.cpp`)
- **Gas density classifier** (see `main/gas_density_classifier.cpp`)
- **AI training mode for gas samples** (see `main/train_AI_bme688.cpp`)

## Hardware Required

- ESP32 development board (tested on ESP32-C6, ESP32-H2, ESP32-P4)
- Bosch BME688 sensor (I2C, address 0x77)
- I2C wiring:
	- SCL: GPIO22
	- SDA: GPIO21
- USB cable for power/programming

## How to Use

### 1. Set Target & Build

Set your chip target (e.g., ESP32-C6):

```sh
idf.py set-target esp32c6
```

Build, flash, and monitor:

```sh
idf.py -p PORT flash monitor
```

### 2. Sensor Reading Example

The main example (`main/read_bme688.cpp`) continuously prints:

- Temperature (°C)
- Pressure (hPa)
- Humidity (%)
- Gas Resistance (KOhms)

### 3. Gas Density Classifier

The classifier (`main/gas_density_classifier.cpp`) loads sample data from `/spiffs/gas_samples.csv` and classifies measured gas resistance against known samples. See code for details.

### 4. AI Training Mode

The training mode (`main/train_AI_bme688.cpp`) lets you record labeled gas resistance samples to `/spiffs/gas_samples.csv` for later classification. Follow serial prompts to record and label samples.

## Example Serial Output

```
I (0) BME688: Temperature: 25.12°C
I (0) BME688: Pressure: 1013.25 hPa
I (0) BME688: Humidity: 45.32 %
I (0) BME688: Gas Resistance: 12.34 KOhms
Measured gas resistance: 12.34 Ohms, Classified as: air
Sample 1: 12.34,air
```

## Python Serial Logger

Use `read_serial_port.py` to log serial output to `output.txt` on your PC. Edit `PORT` as needed.

## Troubleshooting

- **Sensor not detected:** Check I2C wiring and address (default 0x77).
- **SPIFFS errors:** Ensure partition table includes a SPIFFS partition (see `main/partitions.csv`).
- **No output:** Confirm correct COM port and baud rate in `read_serial_port.py`.
- **Build errors:** Check that required components (`bme68x`, `driver`, `spiffs`) are listed in `main/CMakeLists.txt`.

## References

- [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
- [Bosch BME688 Datasheet](https://www.bosch-sensortec.com/products/environmental-sensors/gas-sensors/bme688/)

---
For technical queries, open an [issue](https://github.com/espressif/esp-idf/issues) or contact the project maintainer.
