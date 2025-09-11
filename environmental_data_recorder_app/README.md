
# Environmental Data Recorder App (ESP32)

## Overview
This project is an ESP32-based environmental data logger that reads sensor data from a BME688 sensor and logs it to an SD card. The project is modular, using custom C++ libraries for both the BME688 sensor and SD card access, and is structured for easy extension and maintenance.

## Folder Structure

```
environmental_data_recorder_app/
├── main/
│   ├── environmental_data_recorder_app.cpp   # Main application code
│   ├── CMakeLists.txt
│   └── ...
├── components/
│   ├── bme688_lib/                          # Custom C++ library for BME688 sensor
│   │   ├── include/
│   │   │   └── bme688_lib.h
│   │   ├── bme688_lib.cpp
│   │   └── CMakeLists.txt
│   ├── sdcard_lib/                          # Custom C++ library for SD card access
│   │   ├── include/
│   │   │   └── sdcard_lib.h
│   │   ├── sdcard_lib.cpp
│   │   └── CMakeLists.txt
│   └── bme68x/                              # Bosch BME68x sensor driver (from Bosch)
│       ├── include/
│       │   └── bme68x.h, bme68x_defs.h, ...
│       ├── bme68x.c, ...
│       └── CMakeLists.txt
└── ... (other ESP-IDF project files)
```

## Component Details

### 1. `bme68x` (Bosch)
- **Source:** [Bosch Sensortec BME68x Sensor API](https://github.com/BoschSensortec/BME68x-Sensor-API)
- **Description:**
	- Official C driver from Bosch for the BME680/BME688 environmental sensor.
	- Provides low-level sensor communication, configuration, and data acquisition functions.
	- Used as a dependency by the custom BME688 C++ library.

### 2. `bme688_lib` (Custom)
- **Author:** This project (custom written)
- **Description:**
	- C++ wrapper library for the BME688 sensor, built on top of the Bosch `bme68x` C driver.
	- Handles sensor initialization, configuration, and provides a simple interface for reading measurements.
	- Exposes a `BME688` class with methods like `read_measurement()` for easy use in the main application.

### 3. `sdcard_lib` (Custom)
- **Author:** This project (custom written)
- **Description:**
	- C++ library for SD card access using ESP-IDF's SPI and FATFS APIs.
	- Handles SD card initialization, file/directory operations, and unmounting.
	- Exposes an `SDCard` class with methods like `init()`, `writeFile()`, `createDirectory()`, and `unmount()`.

## Main Application Usage

- The main application (`environmental_data_recorder_app.cpp`) creates objects from the `SDCard` and `BME688` classes.
- It uses these objects to initialize the SD card, create directories, initialize the sensor, read measurements, and log data to the SD card.
- The code is modular and can be easily extended to add new features or change the data logging logic.

## Notes
- The `bme68x` folder is taken directly from Bosch's official [BME68x Sensor API GitHub repository](https://github.com/BoschSensortec/BME68x-Sensor-API).
- The `bme688_lib` and `sdcard_lib` components are custom-written for this project to provide a modern, object-oriented interface for sensor and SD card operations.

---
For more details, see the source code and component headers in the `components/` directory.
