#ifndef MLX90614_LIB_H
#define MLX90614_LIB_H

#include <iostream>
#include "driver/i2c.h"
#define MLX90614_I2C_ADDRESS 0x5A
#define MLX90614_REG_TA 0x06
#define MLX90614_REG_TOBJ1 0x07
#define MLX90614_REG_TOBJ2 0x08

class MLX90614 {
public:
    MLX90614();
    ~MLX90614();
    bool init();
    float readAmbientTempC();
    float readObjectTempC();
private:
    i2c_port_t i2c_master_port;
    bool readRegister(uint8_t reg, uint16_t &value);
    float rawToCelsius(uint16_t raw);
};

#endif // MLX90614_LIB_H
