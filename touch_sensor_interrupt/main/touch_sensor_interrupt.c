#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "freertos/queue.h"
#include <inttypes.h>

/* * ESP-IDF v5.x Internal Sensors Example
 * Includes: Temperature Sensor, Hall Effect, and Capacitive Touch
 */
#include "driver/touch_pad.h"


static const char *TAG = "TOUCH_SENSOR_ONLY";

// We will use Touch Pad 3, which is GPIO 15 on most ESP32 DevKits
#define TOUCH_PAD_NUM    TOUCH_PAD_NUM3 
#define STRONG_TOUCH_THRESH_NOISE 600 // Threshold for detecting a "weak touch"
#define WEAK_TOUCH_THRESH_NOISE 1300 // Threshold for detecting a "strong touch"

// Use on-board BOOT button (GPIO0)
#define BUTTON_GPIO GPIO_NUM_0

static QueueHandle_t gpio_evt_queue = NULL;

// ISR handler for the button GPIO interrupt
// ISR sends the GPIO level (0 pressed, 1 released) to the queue
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t level = gpio_get_level(BUTTON_GPIO);
    xQueueSendFromISR(gpio_evt_queue, &level, NULL);
}

/* forward declaration: read_touch_pad is implemented below but used by measure_task */
void read_touch_pad(void);

// Measurement task: waits for a press event, then measures until a release event.
static void measure_task(void* arg)
{
    uint32_t evt;
    for (;;) {
        // Wait for a button event (press or release)
        if (xQueueReceive(gpio_evt_queue, &evt, portMAX_DELAY) == pdTRUE) {
            // evt == 0 means pressed (BOOT button pulls to GND), evt == 1 means released
            if (evt == 0) {
                ESP_LOGI(TAG, "BOOT pressed, starting measurements");
                // Short debounce: let bounce settle, then drain queue
                vTaskDelay(pdMS_TO_TICKS(50));
                while (xQueueReceive(gpio_evt_queue, &evt, 0) == pdTRUE) {
                    /* drain */
                }
                // Continue measuring until we receive a release event
                // If release detection fails on this board, a subsequent press
                // (level 0) will also stop measurements (toggle behavior).
                for (;;) {
                    read_touch_pad();
                    // Wait for release or next-press event with timeout (measurement interval)
                    if (xQueueReceive(gpio_evt_queue, &evt, pdMS_TO_TICKS(500)) == pdTRUE) {
                        if (evt == 1) {
                            ESP_LOGI(TAG, "BOOT released, stopping measurements");
                            break; // exit measurement loop and go back to waiting for press
                        } else if (evt == 0) {
                            ESP_LOGI(TAG, "BOOT pressed again, toggling stop");
                            break;
                        }
                    }
                }
            }
        }
    }
}

// Function to initialize the touch pad peripheral and configure the specific pad
void init_touch_pad()
{
    // Initialize the touch pad peripheral
    ESP_ERROR_CHECK(touch_pad_init());

    // Set voltage reference for charging/discharging the capacitance
    ESP_ERROR_CHECK(touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V));

    // Configure the specific pad (GPIO 15)
    ESP_ERROR_CHECK(touch_pad_config(TOUCH_PAD_NUM, 0));

    // Perform a filter setup to smooth out noise
    ESP_ERROR_CHECK(touch_pad_filter_start(10));
}

void set_button_interrupt()
{
    // Create a queue to handle GPIO events from the ISR
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    // Configure the button GPIO as input with pull-up and interrupt on any edge
    // so we receive both press (falling, level 0) and release (rising, level 1).
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Install the ISR service and add the handler for the button GPIO
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void*) BUTTON_GPIO));

    // Create the measurement task which waits for press/release events
    xTaskCreate(measure_task, "measure_task", 4096, NULL, 10, NULL);
}

void read_touch_pad()
{
    uint16_t touch_value;
    // We use the filtered read for a more stable value
    touch_pad_read_filtered(TOUCH_PAD_NUM, &touch_value);

    // Print results
    printf("====================================\n");
    if (touch_value < WEAK_TOUCH_THRESH_NOISE && touch_value >= STRONG_TOUCH_THRESH_NOISE) {
        printf("TOUCH (D15)  : %u [TOUCHED WEAKLY.]\n", touch_value);
    }
    else if (touch_value < STRONG_TOUCH_THRESH_NOISE) {
        printf("TOUCH (D15)  : %u [TOUCHED STRONGLY!]\n", touch_value);
    } 
    else {
        printf("TOUCH (D15)  : %u\n", touch_value);
    }
    printf("====================================\n");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Capacitive Touch Sensor...");

    // Initialize the touch pad peripheral
    init_touch_pad();

    // Initialize the button interrupt
    set_button_interrupt();

    /* All work happens in button_task when an interrupt occurs. Suspend this task. */
    vTaskDelete(NULL);
}