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

void sendTestPackage(QwiicRF &loRa) {
    const char* msg = "Hello from ESP-IDF";
    esp_err_t err = loRa.sendPacket((const uint8_t*)msg, strlen(msg));
    if (err == ESP_OK) {
        ESP_LOGI("MAIN", "Sent packet");
    } else {
        ESP_LOGE("MAIN", "Error sending: %d", err);
    }
}

extern "C" void app_main(void) {

    QwiicRF loRa(I2C_PORT, SDA_PIN, SCL_PIN, I2C_FREQ);

    initLoRa(loRa);
    // Configure addressing
    // Set this node's RF address (e.g., 0x02)
    loRa.setRFAddress(0x02);
    // Set the 'paired' destination address to receiver's address (e.g., 0x01)
    loRa.setPairedAddress(0x01);
    
    while (true) {
        // Send to paired address
        sendTestPackage(loRa);
        // Or explicitly to a given address
        const char* msg2 = "Hi 0x01 from 0x02";
        loRa.sendPacketTo(0x01, (const uint8_t*)msg2, strlen(msg2));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}