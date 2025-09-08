#pragma once
#include "driver/gpio.h"

class Button {
public:
    explicit Button(gpio_num_t pin);
    bool is_pressed() const;
private:
    gpio_num_t pin_;
};

class Led {
public:
    explicit Led(gpio_num_t pin);
    void set(bool on);
private:
    gpio_num_t pin_;
};
