#include <string.h>
#include "bme68x.h"
#include "bme68x_defs.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_FREQ_HZ 100000
#define BME68X_ADDR 0x77

class BME688 {
public:
    static constexpr const char* TAG = "BME688";
    BME688() {
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

        dev_addr = BME68X_ADDR;
        dev = {};
        dev.intf = BME68X_I2C_INTF;
        dev.read = bme68x_i2c_read;
        dev.write = bme68x_i2c_write;
        dev.delay_us = bme68x_delay_us;
        dev.intf_ptr = &dev_addr;

        int8_t rslt = bme68x_init(&dev);
        if (rslt != BME68X_OK) {
            ESP_LOGE(TAG, "BME68x initialization failed: %d", rslt);
            ok = false;
            return;
        }

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

    bool read_measurement() {
        if (!ok) return false;
        int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
        if (rslt != BME68X_OK) {
            ESP_LOGE(TAG, "bme68x_set_op_mode failed: %d", rslt);
            return false;
        }
        uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) / 1000 + heatr_conf.heatr_dur;
        vTaskDelay(del_period / portTICK_PERIOD_MS + 1);
        struct bme68x_data data;
        uint8_t n_fields;
        rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
        if (rslt == BME68X_OK && n_fields > 0) {
            int64_t now = esp_timer_get_time() / 1000; // ms
            ESP_LOGI(TAG, "Timestamp: %lld ms", now);
            ESP_LOGI(TAG, "Temperature: %.2f°C", data.temperature);
            ESP_LOGI(TAG, "Pressure: %.2f hPa", data.pressure / 100.0f);
            ESP_LOGI(TAG, "Humidity: %.2f %%", data.humidity);
            ESP_LOGI(TAG, "Gas Resistance: %.2f KOhms", data.gas_resistance / 1000.0);
            return true;
        } else {
            ESP_LOGW(TAG, "No data or error reading BME68x: %d", rslt);
            return false;
        }
    }

    static int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
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

    static int8_t bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
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

    static void bme68x_delay_us(uint32_t period, void *intf_ptr) {
        esp_rom_delay_us(period);
    }

private:
    struct bme68x_dev dev;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    uint8_t dev_addr;
    bool ok = false;

};


extern "C" void app_main() {
    BME688 sensor;
    while (true) {
        sensor.read_measurement();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
