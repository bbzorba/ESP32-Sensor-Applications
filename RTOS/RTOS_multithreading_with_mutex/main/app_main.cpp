#include "button_led.hpp"
#include "bme688_sensor.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

extern "C" void button_task(void* pvParameters) {
    Led* led = static_cast<Led*>(pvParameters);
    Button button(GPIO_NUM_0); // Change pin if needed
    while (true) {
        led->set(button.is_pressed());
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

extern "C" void bme688_task(void* pvParameters) {
    BME688 sensor;
    while (true) {
        float temp = sensor.read_temperature();
        float hum = sensor.read_humidity();
        float pres = sensor.read_pressure();
        float gas_res = sensor.read_gas_resistance();
        printf("[BME688] Temperature: %.2f°C\n", temp);
        printf("[BME688] Humidity: %.2f %%\n", hum);
        printf("[BME688] Pressure: %.2f hPa\n", pres / 100.0f);
        printf("[BME688] Gas Resistance: %.2f Ohms\n", gas_res);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main() {
    static Led led(GPIO_NUM_2);
    xTaskCreate(button_task, "ButtonTask", 2048, &led, 5, nullptr);
    xTaskCreate(bme688_task, "BME688Task", 4096, nullptr, 5, nullptr);
}
