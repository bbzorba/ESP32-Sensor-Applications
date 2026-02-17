#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* * ESP-IDF v5.x Internal Sensors Example
 * Includes: Temperature Sensor, Hall Effect, and Capacitive Touch
 */
#include "driver/touch_pad.h"


static const char *TAG = "TOUCH_SENSOR_ONLY";

// We will use Touch Pad 3, which is GPIO 15 on most ESP32 DevKits
#define TOUCH_PAD_NUM    TOUCH_PAD_NUM3 
#define TOUCH_ON_BOTH_SIDE_THRESH_NOISE 600 // Threshold for detecting a "touch" (lower value = touch)
#define TOUCH_ON_ONE_SIDE_THRESH_NOISE 1300 // Threshold for detecting a "touch" on one side

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Capacitive Touch Sensor...");

    // Initialize the touch pad peripheral
    ESP_ERROR_CHECK(touch_pad_init());

    // Set voltage reference for charging/discharging the capacitance
    ESP_ERROR_CHECK(touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V));

    // Configure the specific pad (GPIO 15)
    ESP_ERROR_CHECK(touch_pad_config(TOUCH_PAD_NUM, 0));

    // Perform a filter setup to smooth out noise
    ESP_ERROR_CHECK(touch_pad_filter_start(10));

    while (1) {
        uint16_t touch_value;
        // We use the filtered read for a more stable value
        touch_pad_read_filtered(TOUCH_PAD_NUM, &touch_value);

        // Print results
        printf("====================================\n");
        if (touch_value < TOUCH_ON_BOTH_SIDE_THRESH_NOISE) { // Adjust this threshold based on your specific hardware and environment
            printf("TOUCH (D15)  : %u [TOUCHED ON BOTH SIDES!]\n", touch_value);
        }
        else if (touch_value < TOUCH_ON_ONE_SIDE_THRESH_NOISE) { // A range for "maybe touched"
            printf("TOUCH (D15)  : %u [TOUCHED ON ONE SIDE!]\n", touch_value);
        } 
        else {
            printf("TOUCH (D15)  : %u\n", touch_value);
        }
        printf("====================================\n");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}