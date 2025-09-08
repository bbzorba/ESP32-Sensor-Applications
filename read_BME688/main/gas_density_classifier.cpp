

#include <stdio.h>
#include <string.h>
#include "bme68x.h"
#include "bme68x_defs.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "esp_rom_sys.h"
#include "esp_spiffs.h"

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

    float read_gas_resistance() {
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


struct Threshold {
    float value;
    char label[32];
};

Threshold thresholds[10];
int num_thresholds = 0;

void init_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        printf("Failed to mount SPIFFS (%d)\n", ret);
    }
}

void load_thresholds() {
    FILE* f = fopen("/spiffs/gas_samples.csv", "r");
    if (!f) {
        printf("Failed to open /spiffs/gas_samples.csv for reading\n");
        return;
    }
    char line[128];
    fgets(line, sizeof(line), f); // skip header
    while (fgets(line, sizeof(line), f)) {
        int sample_num;
        float value;
        char label[32];
        if (sscanf(line, "%d,%f,%31s", &sample_num, &value, label) == 3) {
            thresholds[num_thresholds].value = value;
            strncpy(thresholds[num_thresholds].label, label, sizeof(thresholds[num_thresholds].label));
            num_thresholds++;
            if (num_thresholds >= 10) break;
        }
    }
    fclose(f);
}

const char* classify_gas(float gas_res) {
    // Find closest threshold
    float min_diff = 1e9;
    const char* best_label = "Unknown";
    for (int i = 0; i < num_thresholds; ++i) {
        float diff = fabsf(gas_res - thresholds[i].value);
        if (diff < min_diff) {
            min_diff = diff;
            best_label = thresholds[i].label;
        }
    }
    return best_label;
}


extern "C" void app_main() {
    init_spiffs();
    load_thresholds();
    BME688 sensor;
    printf("Gas density classifier running...\n");
    while (true) {
        float gas_res = sensor.read_gas_resistance();
        const char* gas_type = classify_gas(gas_res);
        printf("Measured gas resistance: %.2f Ohms, Classified as: %s\n", gas_res, gas_type);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}