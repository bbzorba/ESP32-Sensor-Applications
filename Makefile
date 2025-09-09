# Makefile for ESP-IDF project for PowerShell on Windows (V2) & Mac OS

# Use forward slashes for paths.

#Paths for Work PC
#PROJECT_DIR     := $(if $(PROJECT_DIR),$(PROJECT_DIR),D:/baris/personal/personal_projects/ESP32/my_projects/servo_motor)
#IDF_PATH        := $(if $(IDF_PATH),$(IDF_PATH),C:/Users/bzorba.B1-ES/esp/v5.4.2/esp-idf)
#IDF_PYTHON      := $(if $(IDF_PYTHON),$(IDF_PYTHON),C:/Users/bzorba.B1-ES/.espressif/python_env/idf5.4_py3.13_env/Scripts/python.exe)

#Paths for Home PC
PROJECT_DIR     := $(if $(PROJECT_DIR),$(PROJECT_DIR),E:/dev/ESP32/my_Projects/button_LED)
IDF_PATH        := $(if $(IDF_PATH),$(IDF_PATH),C:/Users/Xigmatek/esp/esp-idf)
IDF_PYTHON      := $(if $(IDF_PYTHON),$(IDF_PYTHON),C:/Users/Xigmatek/.espressif/python_env/idf4.2_py3.8_env/Scripts/python.exe)

# #Paths for MacOS
#PROJECT_DIR     := $(if $(PROJECT_DIR),$(PROJECT_DIR),/Users/basakisilzorba/Desktop/ESP32_Projects-main/blink)
#IDF_PATH        := $(if $(IDF_PATH),$(IDF_PATH),/Users/basakisilzorba/esp/esp-idf)
#IDF_PYTHON      := $(if $(IDF_PYTHON),$(IDF_PYTHON),/Users/basakisilzorba/required_idf_tools_path/python_env/idf5.5_py3.13_env/bin/python3)


# Default serial port.
PORT ?= COM7

# Default serial port for Mac (usually /dev/cu.usbserial-*)
#PORT ?= /dev/cu.usbserial-0001


# Register targets that are not files.
.PHONY: all build flash monitor menuconfig clean



# ----------------------------------- Main Command Template -----------------------------------

# Use ESP-IDF's managed Python to run idf.py, ensuring correct dependencies.
IDF_CMD = powershell -NoProfile -ExecutionPolicy Bypass -Command "& { . '$(IDF_PATH)/export.ps1'; Set-Location '$(PROJECT_DIR)'; & '$(IDF_PYTHON)' '$(IDF_PATH)/tools/idf.py' $(1) }"

# MacOS IDF CMD
# IDF_CMD = bash -c ". '$(IDF_PATH)/export.sh'; cd '$(PROJECT_DIR)'; '$(IDF_PYTHON)' '$(IDF_PATH)/tools/idf.py' $(1)"


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
# -------------------------------- End of Main Command Template ------------------------------