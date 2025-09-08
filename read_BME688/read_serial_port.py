import serial
import time

# Configure the serial port
PORT = "COM4"
BAUDRATE = 115200  # Update if needed
OUTPUT_FILE = "output.txt"

try:
    with serial.Serial(PORT, BAUDRATE, timeout=1) as ser, open(OUTPUT_FILE, "a") as f:
        print(f"Listening on {PORT} at {BAUDRATE} baud...")
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode(errors='ignore').strip()
                if line:
                    print(line)
                    f.write(line + "\n")
                    f.flush()
except serial.SerialException as e:
    print(f"[ERROR] Could not open port {PORT}: {e}")
except KeyboardInterrupt:
    print("\n[INFO] Stopped by user.")
