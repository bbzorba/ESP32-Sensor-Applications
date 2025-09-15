#include "mlx90614_lib.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "MLX90614";


MLX90614::MLX90614() : i2c_master_port(I2C_NUM_0) {
    // Constructor implementation
    // Do not install driver here; let init() handle it
}


MLX90614::~MLX90614() {
    // Destructor implementation
    i2c_driver_delete(i2c_master_port);
}


bool MLX90614::init() {
    // Initialization code (minimal, assumes config done elsewhere)
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_21; // Set your SDA pin
    conf.scl_io_num = GPIO_NUM_22; // Set your SCL pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %d", err);
        return false;
    }
    // Only install if not already installed
    static bool installed = false;
    if (!installed) {
        err = i2c_driver_install(i2c_master_port, I2C_MODE_MASTER, 0, 0, 0);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "i2c_driver_install failed: %d", err);
            return false;
        }
        installed = true;
    }
    return true;
}

float MLX90614::readAmbientTempC() {
    // Read ambient temperature in Celsius
    uint16_t raw;
    if (readRegister(MLX90614_REG_TA, raw)) {
        float temp = rawToCelsius(raw);
        return temp - 1.0f; // Example offset for calibration
    }
    return 0.0;
}

float MLX90614::readObjectTempC() {
    // Read object temperature in Celsius
    uint16_t raw;
    if (readRegister(MLX90614_REG_TOBJ1, raw)) {
        float temp = rawToCelsius(raw);
        return temp - 1.0f; // Example offset for calibration
    }
    return 0.0;
}

bool MLX90614::readRegister(uint8_t reg, uint16_t &value) {
    // Read a register from the sensor using repeated start
    uint8_t data[3] = {0};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MLX90614_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    // Do NOT stop here, do repeated start
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MLX90614_I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C reg read failed: %d", ret);
        return false;
    }
    value = (data[1] << 8) | data[0]; // LSB first
    // Optionally log raw data for debug
    ESP_LOGD(TAG, "Read reg 0x%02X: raw=0x%04X", reg, value);
    return true;
}
float MLX90614::rawToCelsius(uint16_t raw) {
    // Convert raw value to Celsius
    return (raw * 0.02) - 273.15;
}
