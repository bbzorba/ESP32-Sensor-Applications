# Makefile for ESP-IDF project for PowerShell on Windows (V2)

# Use forward slashes for paths.
PROJECT_DIR     := $(if $(PROJECT_DIR),$(PROJECT_DIR),E:/dev/ESP32/my_Projects/button_LED)
IDF_PATH        := $(if $(IDF_PATH),$(IDF_PATH),C:/Users/Xigmatek/esp/esp-idf)
IDF_PYTHON      := $(if $(IDF_PYTHON),$(IDF_PYTHON),C:/Users/Xigmatek/.espressif/python_env/idf4.2_py3.8_env/Scripts/python.exe)

# Default serial port.
PORT ?= COM7

# Register targets that are not files.
.PHONY: all build flash monitor menuconfig clean

# --- Main Command Template ---
# This defines a reusable command pattern using a robust PowerShell script block.

# Use ESP-IDF's managed Python to run idf.py, ensuring correct dependencies.
IDF_CMD = powershell -NoProfile -ExecutionPolicy Bypass -Command "& { . '$(IDF_PATH)/export.ps1'; Set-Location '$(PROJECT_DIR)'; & '$(IDF_PYTHON)' '$(IDF_PATH)/tools/idf.py' $(1) }"

# Default target
all: build

# Each target calls the template and passes its specific idf.py command as an argument.
build:
	$(call IDF_CMD, build)

menuconfig:
	$(call IDF_CMD, menuconfig)

flash:
	$(call IDF_CMD, -p $(PORT) flash)

monitor:
	$(call IDF_CMD, -p $(PORT) monitor)

clean:
	$(call IDF_CMD, fullclean)
	