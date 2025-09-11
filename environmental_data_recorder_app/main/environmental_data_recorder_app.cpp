#include "bme688_lib.h"
#include "sdcard_lib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


extern "C" void app_main() {
    // No SNTP or real-time clock initialization
    static int i = 10;

    // Initialize SD card
    SDCard sdCard("/sdcard", 23, 19, 18, 2);
    if (sdCard.init() != ESP_OK) {
        ESP_LOGE("APP", "Failed to initialize SD card");
        sdCard.unmount();
        return;
    }
    ESP_LOGI("APP", "SD card initialized successfully");

    // Create a directory for logs if it doesn't exist
    bool logsDirOk = false;
    if (!sdCard.directoryExists("logs")) {
        sdCard.createDirectory("logs");
        logsDirOk = sdCard.directoryExists("logs");
    } else {
        logsDirOk = true;
    }
    if (!logsDirOk) {
        ESP_LOGW("APP", "Failed to create 'logs' directory, will log to root directory as fallback.");
    } else {
        ESP_LOGI("APP", "'logs' directory exists and will be used for logging.");
    }

    // Initialize BME688 sensor
    BME688 bme688;
    if (!bme688.read_measurement()) {
        ESP_LOGE("APP", "Failed to initialize BME688 sensor");
        sdCard.unmount();
        return;
    }
    ESP_LOGI("APP", "BME688 sensor initialized successfully");

    // Task to read BME688 data and log to SD card
    while (i>0) {
        if (bme688.read_measurement()) {
            float temperature = 0, pressure = 0, humidity = 0, gas_resistance = 0;
            bme688.get_last_measurement(temperature, pressure, humidity, gas_resistance);
            char logLine[256];
            int64_t ms = esp_timer_get_time() / 1000;
            snprintf(logLine, sizeof(logLine), "Timestamp: %lld ms, Temp: %.2f C, Press: %.2f hPa, Hum: %.2f %%, Gas: %.2f KOhms\n", ms, temperature, pressure, humidity, gas_resistance);
            const char* filePath = logsDirOk ? "logs/log.txt" : "log.txt";
            ESP_LOGI("APP", "Writing to file: %s", filePath);
            sdCard.writeFile(filePath, logLine);
            ESP_LOGI("APP", "Logged BME688 data to SD card");
            i--;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    // Unmount SD card before exiting
    sdCard.unmount();
    ESP_LOGI("APP", "SD card unmounted");
}