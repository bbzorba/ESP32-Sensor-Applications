#include "bme688_lib.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define a local tag for logging purposes within this source file.
static const char *TAG = "BME688_LIB";

// Constructor implementation.
BME688::BME688() {
    // Configure and install the I2C driver.
    i2c_config_t i2c_conf;
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = I2C_MASTER_SDA_IO;
    i2c_conf.scl_io_num = I2C_MASTER_SCL_IO;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_conf.clk_flags = 0;
    i2c_param_config(I2C_MASTER_NUM, &i2c_conf);
    i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0);

    // Initialize the BME68x sensor device structure.
    dev_addr = BME68X_ADDR;
    dev = {};
    dev.intf = BME68X_I2C_INTF;
    dev.read = bme68x_i2c_read;
    dev.write = bme68x_i2c_write;
    dev.delay_us = bme68x_delay_us;
    dev.intf_ptr = &dev_addr;

    // Initialize the BME68x sensor.
    int8_t rslt = bme68x_init(&dev);
    if (rslt != BME68X_OK) {
        ESP_LOGE(TAG, "BME68x initialization failed: %d", rslt);
        ok = false;
        return;
    }

    // Configure sensor oversampling settings.
    conf.os_hum = BME68X_OS_2X;
    conf.os_pres = BME68X_OS_4X;
    conf.os_temp = BME68X_OS_8X;
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    rslt = bme68x_set_conf(&conf, &dev);
    if (rslt != BME68X_OK) {
        ESP_LOGE(TAG, "bme68x_set_conf failed: %d", rslt);
        ok = false;
        return;
    }

    // Configure the heater profile for gas resistance measurement.
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &dev);
    if (rslt != BME68X_OK) {
        ESP_LOGE(TAG, "bme68x_set_heatr_conf failed: %d", rslt);
        ok = false;
        return;
    }
    ok = true;
}

// Destructor implementation.
// Cleans up I2C driver resources.
BME688::~BME688() {
    i2c_driver_delete(I2C_MASTER_NUM);
}

// Reads a measurement from the BME688 sensor.
bool BME688::read_measurement() {
    if (!ok) return false;
    
    // Set the sensor to forced mode to perform a single measurement.
    int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
    if (rslt != BME68X_OK) {
        ESP_LOGE(TAG, "bme68x_set_op_mode failed: %d", rslt);
        return false;
    }

    // Get the required delay time for the measurement and heater.
    uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) / 1000 + heatr_conf.heatr_dur;

    // Delay to allow the sensor to complete the measurement.
    vTaskDelay(pdMS_TO_TICKS(del_period) + 1);

    struct bme68x_data data;
    uint8_t n_fields;

    // Read the sensor data.
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
    if (rslt == BME68X_OK && n_fields > 0) {
        int64_t now = esp_timer_get_time() / 1000; // ms
        last_temperature = data.temperature;
        last_pressure = data.pressure / 100.0f;
        last_humidity = data.humidity;
        last_gas_resistance = data.gas_resistance / 1000.0f;
        ESP_LOGI(TAG, "Timestamp: %lld ms", now);
        ESP_LOGI(TAG, "Temperature: %.2f degC", last_temperature);
        ESP_LOGI(TAG, "Pressure: %.2f hPa", last_pressure);
        ESP_LOGI(TAG, "Humidity: %.2f %%", last_humidity);
        ESP_LOGI(TAG, "Gas Resistance: %.2f KOhms", last_gas_resistance);
        return true;
    } else {
        ESP_LOGW(TAG, "No data or error reading BME68x: %d", rslt);
        return false;
    }
}

// Static member function for I2C read, required by the Bosch sensor API.
int8_t BME688::bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_addr = *(uint8_t *)intf_ptr;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, reg_data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return (ret == ESP_OK) ? 0 : -1;
}

// Static member function for I2C write, required by the Bosch sensor API.
int8_t BME688::bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_addr = *(uint8_t *)intf_ptr;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, (uint8_t *)reg_data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return (ret == ESP_OK) ? 0 : -1;
}

// Static member function for microsecond delays, required by the Bosch sensor API.
void BME688::bme68x_delay_us(uint32_t period, void *intf_ptr) {
    esp_rom_delay_us(period);
}
