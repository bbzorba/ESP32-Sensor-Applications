

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class LedBlink {
public:
    static constexpr gpio_num_t LED_GPIO = GPIO_NUM_2;
    LedBlink() {
        gpio_reset_pin(LED_GPIO);
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    }
    void loop() {
        while (true) {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
};

extern "C" void app_main() {
    LedBlink blink;
    blink.loop();
}
