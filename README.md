# ESP32 Embedded Projects

This repository is a collection of the main source codes for various embedded projects developed using the ESP-IDF framework for the ESP32 WROOM 32 module. Each subdirectory within this repository will contain a self-contained ESP-IDF project.

## Getting Started

To build and flash these projects onto your ESP32, you'll need to set up your development environment.

## Prerequisites


###    ESP-IDF (Espressif IoT Development Framework) and Required Tools:

Follow the official Espressif installation guide for your operating system. This will install all required tools, including:

- **Git**: For cloning the repository and managing versions.
- **CMake**: Build system used by ESP-IDF.
- **Ninja**: Fast build tool used by ESP-IDF (required for building projects).
- **Python**: Essential for ESP-IDF scripts and build system.
- **GCC Toolchain**: Cross-compiler for ESP32 (e.g., xtensa-esp32-elf-gcc).
- **Serial Port Driver**: Ensure you have the correct drivers for your ESP32 board's USB-to-serial chip (e.g., CP210x, FTDI).

> **Tip:** The ESP-IDF installer (Windows) or the `install.sh`/`install.bat` scripts (Linux/macOS/Windows) will automatically install CMake, Ninja, Python, and the GCC toolchain for you. You do not need to install these separately unless you want to use your own versions.

**Official ESP-IDF Programming Guide:**
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

###    Required Software/Tools:

Git: For cloning the repository and managing versions.

CMake: Build system used by ESP-IDF.

Python: Essential for ESP-IDF scripts.

Serial Port Driver: Ensure you have the correct drivers for your ESP32 board's USB-to-serial chip (e.g., CP210x, FTDI).

## Setting up a Project


Each project in this repository includes only the source code (main source files and components) for ESP-IDF projects. You should **not clone this repository directly as a project**. Instead, follow these steps to create your own ESP-IDF project and use the provided source code:

### How to Create a New ESP-IDF Project in Visual Studio Code

1. **Install ESP-IDF and ESP-IDF Extension for VS Code:**
    - Follow the official Espressif installation guide to install ESP-IDF and the ESP-IDF VS Code extension.
    - Make sure you have installed the ESP-IDF tools and set up the ESP-IDF Component directory (usually during the ESP-IDF setup process).

2. **Create a New Project:**
    - Open Visual Studio Code.
    - Open the Command Palette (`Ctrl+Shift+P` or `F1`).
    - Type and select `ESP-IDF: Create ESP-IDF Project`.
    - Choose a template (e.g., `blink`, `hello_world`, or `empty` for a blank project).
    - Select the ESP-IDF Component directory (this is the folder where ESP-IDF is installed, e.g., `C:/Users/youruser/esp/esp-idf`).
    - Choose a location and name for your new project.
    - The wizard will generate the project structure for you.


3. **Copy Source Code:**
    - Copy the relevant source files (e.g., `led_blink.cpp`, `read_bme688.cpp`) and any required `components` folders from this repository into the `main` directory (or appropriate location) of your new ESP-IDF project.
    - If your project uses custom components, copy the entire `components` directory and update your project's `CMakeLists.txt` as needed.

4. **Update CMakeLists.txt:**
    - Edit the `main/CMakeLists.txt` file in your project to include the new source files. For example:

          idf_component_register(SRCS "led_blink.cpp" "my_sensor_readings.cpp" INCLUDE_DIRS ".")

5. **Configure, Build, and Flash:**
    - Use the ESP-IDF extension or terminal commands (`idf.py menuconfig`, `idf.py build`, `idf.py flash`) to configure, build, and flash your project.

---
**Using the Provided Makefile:**

As an alternative to running `idf.py` commands directly, you can use the provided Makefile (located at the root of this repository) to build, flash, and monitor your ESP-IDF projects from PowerShell on Windows. Before using it, edit the variables at the top of the Makefile:

    PROJECT_DIR  — Set this to the absolute path of your ESP-IDF project directory (e.g., `D:/baris/personal/personal_projects/ESP32/my_projects/blink`).
    PORT         — Set this to your ESP32's serial port (e.g., `COM4`).
    IDF_PATH     — Set this to your ESP-IDF installation path (e.g., `C:/Users/youruser/esp/esp-idf`).

Once configured, you can run commands like:

    make build      # Builds the project
    make flash      # Flashes the firmware
    make monitor    # Opens the serial monitor
    make menuconfig # Opens the configuration menu

This Makefile is a convenient wrapper for PowerShell users on Windows, but you can always use the standard `idf.py` commands directly if you prefer.
**Note:**
- Do not clone this repository as a full ESP-IDF project. Only copy the source code and components you need into your own ESP-IDF project structure.



### Set up ESP-IDF Environment Variables:
Before building, you need to set up the ESP-IDF environment variables. This is typically done by sourcing the export.sh (Linux/macOS) or export.ps1 (Windows PowerShell) script located in your ESP-IDF installation directory, or by using the ESP-IDF VS Code extension which sets up the environment automatically.

Linux/macOS:
    . $IDF_PATH/export.sh

Windows (PowerShell):
    . $env:IDF_PATH/export.ps1

(Replace $IDF_PATH with your actual ESP-IDF installation path if it's not already set as an environment variable.)

If you use the ESP-IDF VS Code extension, the environment will be set up for you automatically when you open a project folder.

### Configure the Project (Optional but Recommended):
You can configure project-specific settings (like serial port, Wi-Fi credentials, etc.) using menuconfig:

    idf.py menuconfig

### Build the Project:
Compile the source code:


    idf.py build


Alternatively, if you are using the provided Makefile (now located at the root of the repository and shared by all projects):

    make

**Important:**

- Before running any `make` command, you must edit the `Makefile` and set the following variables at the top of the file:
    - `PROJECT_DIR` — Set this to the absolute path of the project you want to build (e.g., `D:/baris/personal/personal_projects/ESP32/my_projects/blink`).
    - `PORT` — Set this to the serial port your ESP32 is connected to (e.g., `COM4`).
    - `IDF_PATH` — Set this to your ESP-IDF installation path (e.g., `C:/Users/youruser/esp/esp-idf`).

The Makefile requires absolute paths and cannot automatically detect your project or ESP-IDF installation directory. Each user must update these variables to match their own system.

### Flash the Project to ESP32:
Once built, flash the firmware to your ESP32 board. Ensure your ESP32 is connected and the correct serial port is selected (either in menuconfig or as PORT in your Makefile).

    idf.py -p /dev/ttyUSB0 flash  # Linux/macOS example
    idf.py -p COM4 flash         # Windows example

If using the Makefile:

    make flash

(Replace /dev/ttyUSB0 or COM4 with your actual ESP32's serial port.)

### Monitor Serial Output (Optional):
To see the output from your ESP32, you can use the monitor:

    idf.py -p /dev/ttyUSB0 monitor # Linux/macOS example
    idf.py -p COM4 monitor         # Windows example

If using the Makefile:

    make monitor

## Project Structure and Customization

Each project directory will typically contain:

main/: Contains the main application source code.

CMakeLists.txt: CMake build script for the project.

sdkconfig: Project configuration generated by menuconfig.

## Important Note for Developers:

When adding new source files or changing the main build file for a project, you must modify the CMakeLists.txt file located inside the main directory of that specific project. This file specifies which source files are included in the build.

For example, to include led_blink.cpp in your build, your main/CMakeLists.txt might look like this:
CMake

    idf_component_register(SRCS "led_blink.cpp"
                       INCLUDE_DIRS ".")

If you add a new file named my_sensor_readings.cpp, you would update it to:
CMake

    idf_component_register(SRCS "led_blink.cpp"
                            "my_sensor_readings.cpp"
                       INCLUDE_DIRS ".")