#ifndef BME688_LIB_H
#define BME688_LIB_H

// Core BME68x sensor library headers
#include "bme68x.h"
#include "bme68x_defs.h"

// ESP-IDF specific headers
#include "esp_log.h"
#include "driver/i2c.h"
#include "esp_timer.h"

// FreeRTOS header for vTaskDelay
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define constants for the I2C bus and BME688 sensor address
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_FREQ_HZ 100000
#define BME68X_ADDR 0x77

/**
 * @class BME688
 * @brief A C++ class to encapsulate BME688 sensor communication and data reading.
 * * This class handles the I2C initialization, sensor configuration, and
 * measurement reading for the BME688 sensor using the official Bosch
 * sensor API.
 */
class BME688 {
public:
    // Last measurement values
    float last_temperature = 0;
    float last_pressure = 0;
    float last_humidity = 0;
    float last_gas_resistance = 0;
    static constexpr const char* TAG = "BME688";

    /**
     * @brief Constructor for the BME688 class.
     * Initializes the I2C bus and configures the BME68x sensor.
     */
    BME688();

    /**
     * @brief Destructor for the BME688 class.
     * Cleans up the I2C driver resources.
     */
    ~BME688();

    /**
     * @brief Reads a measurement from the BME688 sensor.
     * Sets the sensor to forced mode, reads the data, and logs the results.
     * @return true if the measurement was successful, false otherwise.
     */
    bool read_measurement();
    void get_last_measurement(float &temperature, float &pressure, float &humidity, float &gas_resistance) const {
        temperature = last_temperature;
        pressure = last_pressure;
        humidity = last_humidity;
        gas_resistance = last_gas_resistance;
    }

private:
    /**
     * @brief I2C read function for the BME68x API.
     * This function is a static member and is passed to the Bosch API.
     */
    static int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

    /**
     * @brief I2C write function for the BME68x API.
     * This function is a static member and is passed to the Bosch API.
     */
    static int8_t bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);

    /**
     * @brief Delay function for the BME68x API.
     * This function is a static member and is passed to the Bosch API.
     */
    static void bme68x_delay_us(uint32_t period, void *intf_ptr);

    struct bme68x_dev dev;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    uint8_t dev_addr;
    bool ok;
};

#endif // BME688_LIB_H
