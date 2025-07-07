
#include "bme68x.h"
#include "bme68x_defs.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include <string.h>
#include "esp_rom_sys.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_FREQ_HZ 100000
#define BME68X_ADDR 0x77

static const char *TAG = "BME688";

// I2C read function for BME68x
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

// I2C write function for BME68x
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

// Delay function for BME68x (in microseconds)
static void bme68x_delay_us(uint32_t period, void *intf_ptr) {
    esp_rom_delay_us(period);
}

// I2C bus scan function
static void i2c_scan() {
    ESP_LOGI(TAG, "Scanning I2C bus...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address 0x%02X", addr);
        }
    }
}

void app_main() {
    ESP_LOGI(TAG, "Initializing I2C...");

    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0
    };
    i2c_param_config(I2C_MASTER_NUM, &i2c_conf);
    i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0);

    // Scan I2C bus and print found addresses
    i2c_scan();

    ESP_LOGI(TAG, "Initializing BME68x...");
    struct bme68x_dev dev = {0};
    int8_t rslt;
    uint8_t dev_addr = BME68X_ADDR;

    dev.intf = BME68X_I2C_INTF;
    dev.read = bme68x_i2c_read;
    dev.write = bme68x_i2c_write;
    dev.delay_us = bme68x_delay_us;
    dev.intf_ptr = &dev_addr;

    rslt = bme68x_init(&dev);
    if (rslt != BME68X_OK) {
        ESP_LOGE(TAG, "BME68x initialization failed: %d", rslt);
        return;
    }

    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    struct bme68x_data data;
    uint8_t n_fields;

    // Set sensor settings (standard example)
    conf.os_hum = BME68X_OS_2X;
    conf.os_pres = BME68X_OS_4X;
    conf.os_temp = BME68X_OS_8X;
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    rslt = bme68x_set_conf(&conf, &dev);
    if (rslt != BME68X_OK) {
        ESP_LOGE(TAG, "bme68x_set_conf failed: %d", rslt);
        return;
    }

    // Set heater config (optional, for gas sensor)
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300; // 300°C
    heatr_conf.heatr_dur = 100;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &dev);
    if (rslt != BME68X_OK) {
        ESP_LOGE(TAG, "bme68x_set_heatr_conf failed: %d", rslt);
        return;
    }

    while (1) {
        // Set forced mode
        rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
        if (rslt != BME68X_OK) {
            ESP_LOGE(TAG, "bme68x_set_op_mode failed: %d", rslt);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        // Wait for measurement to complete
        uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) / 1000 + heatr_conf.heatr_dur;
        vTaskDelay(del_period / portTICK_PERIOD_MS + 1);

        // Read data
        rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
        if (rslt == BME68X_OK && n_fields > 0) {
            ESP_LOGI(TAG, "Temperature: %.2f°C", data.temperature);
            ESP_LOGI(TAG, "Pressure: %.2f hPa", data.pressure / 100.0f);
            ESP_LOGI(TAG, "Humidity: %.2f %%", data.humidity);
            ESP_LOGI(TAG, "Gas Resistance: %.2f KOhms", data.gas_resistance / 1000.0);
        } else {
            ESP_LOGW(TAG, "No data or error reading BME68x: %d", rslt);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
