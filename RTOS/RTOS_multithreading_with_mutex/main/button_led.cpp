#include "button_led.hpp"
#include "driver/gpio.h"

Button::Button(gpio_num_t pin) : pin_(pin) {
    gpio_reset_pin(pin_);
    gpio_set_direction(pin_, GPIO_MODE_INPUT);
    gpio_set_pull_mode(pin_, GPIO_PULLUP_ONLY);
}

bool Button::is_pressed() const {
    return gpio_get_level(pin_) == 0;
}

Led::Led(gpio_num_t pin) : pin_(pin) {
    gpio_reset_pin(pin_);
    gpio_set_direction(pin_, GPIO_MODE_OUTPUT);
}

void Led::set(bool on) {
    gpio_set_level(pin_, on ? 1 : 0);
}
