#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include <vector>
#include <cstring>
#include "qwiicrf.h"

#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22
#define I2C_PORT I2C_NUM_0
#define I2C_FREQ 100000  // 100 kHz

void initLoRa(QwiicRF &loRa) {
    esp_err_t err = loRa.init();
    if (err != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to init QwiicRF: %d", err);
        while (true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
}

void listenPackages(QwiicRF &loRa) {
    while (true) {
        // Poll quickly, avoid log spam, and always yield to keep WDT happy
        size_t avail = 0;
        esp_err_t err = loRa.packetAvailable(&avail, pdMS_TO_TICKS(5));
        if (err == ESP_OK && avail > 0) {
            std::vector<uint8_t> buf(avail);
            size_t got = 0;
            err = loRa.readPacket(buf.data(), buf.size(), &got, pdMS_TO_TICKS(20));
            if (err == ESP_OK && got > 0) {
                // Filter obvious noise: all 0xFF bytes or common echo pattern [0x03, 0xFF, 0xFF]
                bool all_ff = true;
                for (size_t i = 0; i < got; ++i) { if (buf[i] != 0xFF) { all_ff = false; break; } }
                bool echo_pattern = (got == 3 && buf[0] == 0x03 && buf[1] == 0xFF && buf[2] == 0xFF);
                if (all_ff || echo_pattern) {
                    // Treat as no data; brief backoff to reduce bus churn
                    vTaskDelay(pdMS_TO_TICKS(5));
                    continue;
                }
                // Print as hex for robustness (payload may be binary)
                char line[256];
                int off = snprintf(line, sizeof(line), "[%d]", (int)got);
                for (size_t i = 0; i < got && off + 3 < (int)sizeof(line); ++i) {
                    off += snprintf(line + off, sizeof(line) - off, " %02X", buf[i]);
                }
                ESP_LOGI("MAIN", "Received packet %s", line);
            } else if (err != ESP_OK) {
                ESP_LOGW("MAIN", "Error reading packet: %d", err);
            }
        } else if (err != ESP_OK) {
            ESP_LOGW("MAIN", "Error checking availability: %d", err);
        }

        // Yield briefly to allow idle tasks to run and prevent WDT
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

extern "C" void app_main(void) {

    QwiicRF loRa(I2C_PORT, SDA_PIN, SCL_PIN, I2C_FREQ);

    initLoRa(loRa);
    // Optional: set this node's RF address (e.g., 0x01)
    loRa.setRFAddress(0x01);
    listenPackages(loRa);
}