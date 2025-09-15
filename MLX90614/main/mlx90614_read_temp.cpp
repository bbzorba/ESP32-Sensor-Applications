#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/temperature_sensor.h"
#include "mlx90614_lib.h"

static const char *TAG = "mlx90614_read_temp";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Read ambient and object temperatures with MLX90614");

    MLX90614 mlx90614;
    if (!mlx90614.init()) {
        ESP_LOGE(TAG, "Failed to initialize MLX90614");
        return;
    }

    while(1) {
        ESP_LOGI(TAG, "Read temperature");
        float ambientTemp = mlx90614.readAmbientTempC();
        float objectTemp = mlx90614.readObjectTempC();
        ESP_LOGI(TAG, "Ambient Temperature: %.2f °C", ambientTemp);
        ESP_LOGI(TAG, "Object Temperature: %.2f °C", objectTemp);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
