#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class Button {
public:
    explicit Button(gpio_num_t pin) : pin_(pin) {
        gpio_reset_pin(pin_);
        gpio_set_direction(pin_, GPIO_MODE_INPUT);
        gpio_set_pull_mode(pin_, GPIO_PULLUP_ONLY);
    }
    bool is_pressed() const {
        return gpio_get_level(pin_) == 0; // pressed = logic low
    }
private:
    gpio_num_t pin_;
};

class Led {
public:
    explicit Led(gpio_num_t pin) : pin_(pin) {
        gpio_reset_pin(pin_);
        gpio_set_direction(pin_, GPIO_MODE_OUTPUT);
    }
    void set(bool on) {
        gpio_set_level(pin_, on ? 1 : 0);
    }
private:
    gpio_num_t pin_;
};

extern "C" void app_main() {
    Led led(GPIO_NUM_2);
    Button button(GPIO_NUM_0); // Change if you use a different button pin
    while (true) {
        led.set(button.is_pressed());
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
