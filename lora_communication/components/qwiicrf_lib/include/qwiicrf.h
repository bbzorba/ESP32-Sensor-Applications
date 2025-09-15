#ifndef QWIICRF_H
#define QWIICRF_H

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h" // for TickType_t
#include <cstdint>

class QwiicRF {
public:
    // Default I2C address
    static constexpr uint8_t DEFAULT_ADDR = 0x35;

    // Constructor: you supply I2C port, SDA/SCL pins, maybe clock speed
    QwiicRF(i2c_port_t i2c_port, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz = 100000);

    esp_err_t init();  // initialise I2C & module

    // Send a packet
    // data: buffer of bytes to send
    // Uses the module's paired address (set via setPairedAddress)
    esp_err_t sendPacket(const uint8_t *data, size_t len, TickType_t ticks_to_wait = portMAX_DELAY);

    // Send to a specific RF address
    esp_err_t sendPacketTo(uint8_t rf_addr, const uint8_t *data, size_t len, TickType_t ticks_to_wait = portMAX_DELAY);

    // Check if a packet is available
    // Returns number of bytes available, or 0 if none, or error <0
    esp_err_t packetAvailable(size_t *out_len, TickType_t ticks_to_wait = portMAX_DELAY);

    // Read a packet
    // Reads up to buffer_len bytes into buffer; out_len gets the actual number of bytes
    esp_err_t readPacket(uint8_t *buffer, size_t buffer_len, size_t *out_len, TickType_t ticks_to_wait = portMAX_DELAY);

    // Settings
    esp_err_t setRFAddress(uint8_t addr, TickType_t ticks_to_wait = portMAX_DELAY);
    esp_err_t setPairedAddress(uint8_t addr, TickType_t ticks_to_wait = portMAX_DELAY);

private:
    i2c_port_t _i2c_port;
    gpio_num_t _sda_pin;
    gpio_num_t _scl_pin;
    uint32_t _clk_speed_hz;
    uint8_t _device_address;

    // Internal helper: write command + data
    esp_err_t i2cWrite(const uint8_t *data, size_t len, TickType_t ticks_to_wait);

    // Internal helper: read data
    esp_err_t i2cRead(uint8_t *data, size_t len, TickType_t ticks_to_wait);
};

#endif // QWIICRF_H