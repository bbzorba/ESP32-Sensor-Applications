#include "bme688_sensor.hpp"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_FREQ_HZ 100000
#define BME68X_ADDR 0x77

BME688::BME688() {
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
        ok = false;
        return;
    }

    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &dev);
    if (rslt != BME68X_OK) {
        ok = false;
        return;
    }
    ok = true;
}


float BME688::read_gas_resistance() {
    if (!ok) return -1.0f;
    int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
    if (rslt != BME68X_OK) return -1.0f;
    uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) / 1000 + heatr_conf.heatr_dur;
    vTaskDelay(del_period / portTICK_PERIOD_MS + 1);
    struct bme68x_data data;
    uint8_t n_fields;
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
    if (rslt == BME68X_OK && n_fields > 0) {
        return data.gas_resistance;
    }
    return -1.0f;
}

float BME688::read_temperature() {
    if (!ok) return -1.0f;
    int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
    if (rslt != BME68X_OK) return -1.0f;
    uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) / 1000 + heatr_conf.heatr_dur;
    vTaskDelay(del_period / portTICK_PERIOD_MS + 1);
    struct bme68x_data data;
    uint8_t n_fields;
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
    if (rslt == BME68X_OK && n_fields > 0) {
        return data.temperature;
    }
    return -1.0f;
}

float BME688::read_humidity() {
    if (!ok) return -1.0f;
    int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
    if (rslt != BME68X_OK) return -1.0f;
    uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) / 1000 + heatr_conf.heatr_dur;
    vTaskDelay(del_period / portTICK_PERIOD_MS + 1);
    struct bme68x_data data;
    uint8_t n_fields;
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
    if (rslt == BME68X_OK && n_fields > 0) {
        return data.humidity;
    }
    return -1.0f;
}

float BME688::read_pressure() {
    if (!ok) return -1.0f;
    int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
    if (rslt != BME68X_OK) return -1.0f;
    uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) / 1000 + heatr_conf.heatr_dur;
    vTaskDelay(del_period / portTICK_PERIOD_MS + 1);
    struct bme68x_data data;
    uint8_t n_fields;
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
    if (rslt == BME68X_OK && n_fields > 0) {
        return data.pressure;
    }
    return -1.0f;
}

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

void BME688::bme68x_delay_us(uint32_t period, void *intf_ptr) {
    esp_rom_delay_us(period);
}
