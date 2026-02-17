# Makefile for ESP-IDF project for PowerShell on Windows (V2) & Mac OS

# Use forward slashes for paths.

# OS detection and shell configuration
# On Windows, the environment variable OS is typically set to Windows_NT
IS_WINDOWS := $(filter Windows_NT,$(OS))
ifeq ($(IS_WINDOWS),Windows_NT)
# Force GNU Make to use PowerShell for executing recipe lines on Windows to avoid sh/cmd variable expansion issues
SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -ExecutionPolicy Bypass -Command
else
# Use bash on macOS/Linux and enable pipefail for better error reporting
SHELL := /bin/bash
.SHELLFLAGS := -o pipefail -c
endif

#Paths for Work PC
PROJECT_DIR     := $(if $(PROJECT_DIR),$(PROJECT_DIR),D:/baris/personal/personal_projects/ESP32/my_projects/builtin_touch_sensor)
IDF_PATH        := $(if $(IDF_PATH),$(IDF_PATH),C:/Users/bzorba.B1-ES/esp/v5.4.2/esp-idf)
IDF_PYTHON      := $(if $(IDF_PYTHON),$(IDF_PYTHON),C:/Users/bzorba.B1-ES/.espressif/python_env/idf5.4_py3.13_env/Scripts/python.exe)

#Paths for Home PC
#PROJECT_DIR     := $(if $(PROJECT_DIR),$(PROJECT_DIR),E:/dev/ESP32/my_Projects/servo_motor)
#IDF_PATH        := $(if $(IDF_PATH),$(IDF_PATH),C:/Users/Xigmatek/esp/esp-idf)
#IDF_PYTHON      := $(if $(IDF_PYTHON),$(IDF_PYTHON),C:/Users/Xigmatek/.espressif/python_env/idf4.2_py3.8_env/Scripts/python.exe)

# #Paths for MacOS
#PROJECT_DIR     := $(if $(PROJECT_DIR),$(PROJECT_DIR),/Users/basakisilzorba/Desktop/ESP32_Projects-main/blink)
#IDF_PATH        := $(if $(IDF_PATH),$(IDF_PATH),/Users/basakisilzorba/esp/esp-idf)
#IDF_PYTHON      := $(if $(IDF_PYTHON),$(IDF_PYTHON),/Users/basakisilzorba/required_idf_tools_path/python_env/idf5.5_py3.13_env/bin/python3)


# Default serial port.
PORT ?= COM3

# Default serial port for Mac (usually /dev/cu.usbserial-*)
#PORT ?= /dev/cu.usbserial-0001


# Register targets that are not files.
.PHONY: all build flash flash-auto flashmonitor flashmonitor-auto monitor monitor-auto menuconfig clean



# ----------------------------------- Main Command Template -----------------------------------

ifeq ($(IS_WINDOWS),Windows_NT)
# Use ESP-IDF's managed Python to run idf.py (Windows/PowerShell)
IDF_CMD = powershell -NoProfile -ExecutionPolicy Bypass -Command "& { . '$(IDF_PATH)/export.ps1'; Set-Location '$(PROJECT_DIR)'; & '$(IDF_PYTHON)' '$(IDF_PATH)/tools/idf.py' $(1) }"
else
# Use ESP-IDF's managed Python to run idf.py (macOS/Linux/Bash)
IDF_CMD = bash -lc ". '$(IDF_PATH)/export.sh'; cd '$(PROJECT_DIR)'; '$(IDF_PYTHON)' '$(IDF_PATH)/tools/idf.py' $(1)"
endif


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

flashmonitor:
	$(call IDF_CMD, -p $(PORT) flash monitor)

ifeq ($(IS_WINDOWS),Windows_NT)
# Auto-detect serial port on Windows and flash
flash-auto:
	$$ErrorActionPreference='Stop'; . "$(IDF_PATH)/export.ps1"; $$pnp=$$null; try { $$pnp=Get-PnpDevice -Class Ports -ErrorAction Stop } catch { }; $$port=$$null; if ($$pnp) { $$port = ($$pnp | Where-Object { $$_.FriendlyName -match 'CP210|USB Serial|CH340|Silicon|FTDI' } | Select-Object -ExpandProperty FriendlyName | ForEach-Object { $$_ -replace '.*\((COM\d+)\).*', '$$1' } | Select-Object -First 1) }; if (-not $$port) { $$port = ([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object { [int](($$_ -replace 'COM','')) } -Descending | Select-Object -First 1) }; if (-not $$port) { Write-Error 'No serial port detected'; exit 1 }; Set-Location "$(PROJECT_DIR)"; & "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p $$port flash

# Auto-detect serial port on Windows and open monitor
monitor-auto:
	$$ErrorActionPreference='Stop'; . "$(IDF_PATH)/export.ps1"; $$pnp=$$null; try { $$pnp=Get-PnpDevice -Class Ports -ErrorAction Stop } catch { }; $$port=$$null; if ($$pnp) { $$port = ($$pnp | Where-Object { $$_.FriendlyName -match 'CP210|USB Serial|CH340|Silicon|FTDI' } | Select-Object -ExpandProperty FriendlyName | ForEach-Object { $$_ -replace '.*\((COM\d+)\).*', '$$1' } | Select-Object -First 1) }; if (-not $$port) { $$port = ([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object { [int](($$_ -replace 'COM','')) } -Descending | Select-Object -First 1) }; if (-not $$port) { Write-Error 'No serial port detected'; exit 1 }; Set-Location "$(PROJECT_DIR)"; & "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p $$port monitor; exit 0

flashmonitor-auto:
	$$ErrorActionPreference='Stop'; . "$(IDF_PATH)/export.ps1"; $$pnp=$$null; try { $$pnp=Get-PnpDevice -Class Ports -ErrorAction Stop } catch { }; $$port=$$null; if ($$pnp) { $$port = ($$pnp | Where-Object { $$_.FriendlyName -match 'CP210|USB Serial|CH340|Silicon|FTDI' } | Select-Object -ExpandProperty FriendlyName | ForEach-Object { $$_ -replace '.*\((COM\d+)\).*', '$$1' } | Select-Object -First 1) }; if (-not $$port) { $$port = ([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object { [int](($$_ -replace 'COM','')) } -Descending | Select-Object -First 1) }; if (-not $$port) { Write-Error 'No serial port detected'; exit 1 }; Set-Location "$(PROJECT_DIR)"; & "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p $$port flash; if ($$LASTEXITCODE -ne 0) { exit $$LASTEXITCODE }; & "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p $$port monitor; exit 0
else
# Auto-detect serial port on macOS/Linux and flash
flash-auto:
	set -e; \
	. "$(IDF_PATH)/export.sh"; \
	port=""; \
	for pat in "/dev/cu.SLAB_USBtoUART" "/dev/cu.usbserial*" "/dev/cu.usbmodem*" "/dev/cu.wchusbserial*"; do \
		for dev in $$pat; do if [ -e "$$dev" ]; then port="$$dev"; break; fi; done; \
		[ -n "$$port" ] && break; \
	done; \
	if [ -z "$$port" ]; then port=$$(ls /dev/cu.* 2>/dev/null | grep -Ei 'usb|SLAB|wch|modem|serial' | head -n1); fi; \
	if [ -z "$$port" ]; then echo "No serial port detected" >&2; exit 1; fi; \
	cd "$(PROJECT_DIR)"; "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p "$$port" flash

# Auto-detect serial port on macOS/Linux and open monitor
monitor-auto:
	set -e; \
	. "$(IDF_PATH)/export.sh"; \
	port=""; \
	for pat in "/dev/cu.SLAB_USBtoUART" "/dev/cu.usbserial*" "/dev/cu.usbmodem*" "/dev/cu.wchusbserial*"; do \
		for dev in $$pat; do if [ -e "$$dev" ]; then port="$$dev"; break; fi; done; \
		[ -n "$$port" ] && break; \
	done; \
	if [ -z "$$port" ]; then port=$$(ls /dev/cu.* 2>/dev/null | grep -Ei 'usb|SLAB|wch|modem|serial' | head -n1); fi; \
	if [ -z "$$port" ]; then echo "No serial port detected" >&2; exit 1; fi; \
	cd "$(PROJECT_DIR)"; "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p "$$port" monitor

flashmonitor-auto:
	set -e; \
	. "$(IDF_PATH)/export.sh"; \
	port=""; \
	for pat in "/dev/cu.SLAB_USBtoUART" "/dev/cu.usbserial*" "/dev/cu.usbmodem*" "/dev/cu.wchusbserial*"; do \
		for dev in $$pat; do if [ -e "$$dev" ]; then port="$$dev"; break; fi; done; \
		[ -n "$$port" ] && break; \
	done; \
	if [ -z "$$port" ]; then port=$$(ls /dev/cu.* 2>/dev/null | grep -Ei 'usb|SLAB|wch|modem|serial' | head -n1); fi; \
	if [ -z "$$port" ]; then echo "No serial port detected" >&2; exit 1; fi; \
	cd "$(PROJECT_DIR)"; "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p "$$port" flash && "$(IDF_PYTHON)" "$(IDF_PATH)/tools/idf.py" -p "$$port" monitor
endif

clean:
	$(call IDF_CMD, fullclean)
# -------------------------------- End of Main Command Template ------------------------------