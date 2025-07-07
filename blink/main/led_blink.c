#include <stdio.h>
#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_2

void app_main(void)
{
    // Configure the GPIO pin
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(LED_GPIO, 1);  // LED ON
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(LED_GPIO, 0);  // LED OFF
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
