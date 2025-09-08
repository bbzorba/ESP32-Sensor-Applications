#pragma once
#include "bme68x.h"
#include "bme68x_defs.h"
#include "driver/i2c.h"
#include "esp_rom_sys.h"

class BME688 {
public:
    BME688();
    float read_gas_resistance();
    float read_temperature();
    float read_humidity();
    float read_pressure();
private:
    struct bme68x_dev dev;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    uint8_t dev_addr;
    bool ok = false;
    static int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static int8_t bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static void bme68x_delay_us(uint32_t period, void *intf_ptr);
};
