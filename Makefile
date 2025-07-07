# Makefile for ESP-IDF project for PowerShell on Windows (V2)

# Use forward slashes for paths.
PROJECT_DIR     := D:/baris/personal/personal_projects/ESP32/my_projects/read_bme688
IDF_PATH        := C:/Users/bzorba.B1-ES/esp/v5.4.2/esp-idf

# Default serial port.
PORT ?= COM4

# Register targets that are not files.
.PHONY: all build flash monitor menuconfig clean

# --- Main Command Template ---
# This defines a reusable command pattern using a robust PowerShell script block.
# We use 'export.ps1' (the PowerShell version of the export script)
# and PowerShell's native 'Set-Location' (instead of 'cd').
# Commands inside the block are separated by semicolons ';'.
# The '$(1)' is a placeholder for the specific idf.py arguments.
IDF_CMD = powershell -NoProfile -ExecutionPolicy Bypass -Command "& { . '$(IDF_PATH)/export.ps1'; Set-Location '$(PROJECT_DIR)'; idf.py $(1) }"

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