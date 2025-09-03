#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


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
    while (true) {
        led.set(true);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        led.set(false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
