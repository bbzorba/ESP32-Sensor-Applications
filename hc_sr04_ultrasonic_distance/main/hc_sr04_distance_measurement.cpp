#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_private/esp_clk.h"
#include "driver/mcpwm_cap.h"
#include "driver/gpio.h"

const static char *TAG = "example";


#define HC_SR04_TRIG_GPIO  12
#define HC_SR04_ECHO_GPIO  13

class HCSR04 {
public:
    HCSR04(int trigPin, int echoPin)
        : _trigPin(trigPin), _echoPin(echoPin), cap_timer(nullptr), cap_chan(nullptr) {}

    static bool hc_sr04_echo_callback(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data)
    {
        static uint32_t cap_val_begin_of_sample = 0;
        static uint32_t cap_val_end_of_sample = 0;
        TaskHandle_t task_to_notify = (TaskHandle_t)user_data;
        BaseType_t high_task_wakeup = pdFALSE;

        if (edata->cap_edge == MCPWM_CAP_EDGE_POS) {
            cap_val_begin_of_sample = edata->cap_value;
            cap_val_end_of_sample = cap_val_begin_of_sample;
        } else {
            cap_val_end_of_sample = edata->cap_value;
            uint32_t tof_ticks = cap_val_end_of_sample - cap_val_begin_of_sample;
            xTaskNotifyFromISR(task_to_notify, tof_ticks, eSetValueWithOverwrite, &high_task_wakeup);
        }
        return high_task_wakeup == pdTRUE;
    }

    void gen_trig_output()
    {
        gpio_set_level((gpio_num_t)_trigPin, 1);
        esp_rom_delay_us(10);
        gpio_set_level((gpio_num_t)_trigPin, 0);
    }

    void InitCapture()
    {
        ESP_LOGI(TAG, "Install capture timer");
        mcpwm_capture_timer_config_t cap_conf = {};
        cap_conf.group_id = 0;
        cap_conf.clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT;
        ESP_ERROR_CHECK(mcpwm_new_capture_timer(&cap_conf, &cap_timer));

        ESP_LOGI(TAG, "Install capture channel");
        mcpwm_capture_channel_config_t cap_ch_conf = {};
        cap_ch_conf.gpio_num = _echoPin;
        cap_ch_conf.prescale = 1;
        cap_ch_conf.flags.neg_edge = true;
        cap_ch_conf.flags.pos_edge = true;
    // Enable pull-up for ECHO pin, as in working C code
    cap_ch_conf.flags.pull_up = true;
        ESP_ERROR_CHECK(mcpwm_new_capture_channel(cap_timer, &cap_ch_conf, &cap_chan));
    }

    void RegisterCallback(TaskHandle_t cur_task)
    {
        ESP_LOGI(TAG, "Register capture callback");
        mcpwm_capture_event_callbacks_t cbs = {
            .on_cap = hc_sr04_echo_callback,
        };
        ESP_ERROR_CHECK(mcpwm_capture_channel_register_event_callbacks(cap_chan, &cbs, cur_task));
    }

    void EnableCapture()
    {
        ESP_LOGI(TAG, "Enable capture channel");
        ESP_ERROR_CHECK(mcpwm_capture_channel_enable(cap_chan));
    }

    void ConfigureTrigPin()
    {
        ESP_LOGI(TAG, "Configure Trig pin");
        gpio_config_t io_conf = {};
        io_conf.pin_bit_mask = 1ULL << _trigPin;
        io_conf.mode = GPIO_MODE_OUTPUT;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
        ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)_trigPin, 0));
    }

    void EnableAndStartTimer()
    {
        ESP_LOGI(TAG, "Enable and start capture timer");
        ESP_ERROR_CHECK(mcpwm_capture_timer_enable(cap_timer));
        ESP_ERROR_CHECK(mcpwm_capture_timer_start(cap_timer));
    }

    void SampleAndLogDistance(TaskHandle_t cur_task)
    {
        uint32_t tof_ticks;
        gen_trig_output();
        if (xTaskNotifyWait(0x00, ULONG_MAX, &tof_ticks, pdMS_TO_TICKS(1000)) == pdTRUE) {
            float pulse_width_us = tof_ticks * (1000000.0 / esp_clk_apb_freq());
            if (pulse_width_us > 35000) {
                // out of range
                ESP_LOGW(TAG, "Echo pulse too long or out of range");
                return;
            }
            float distance = (float)pulse_width_us / 58;
            ESP_LOGI(TAG, "Measured distance: %.2fcm", distance);
        } else {
            ESP_LOGW(TAG, "No echo received (timeout). Check wiring and sensor.");
        }
    }

private:
    int _trigPin;
    int _echoPin;
    mcpwm_cap_timer_handle_t cap_timer;
    mcpwm_cap_channel_handle_t cap_chan;
};

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "HC-SR04 example");

    HCSR04 hcsr04(HC_SR04_TRIG_GPIO, HC_SR04_ECHO_GPIO);
    hcsr04.ConfigureTrigPin();
    hcsr04.InitCapture();
    hcsr04.RegisterCallback(xTaskGetCurrentTaskHandle());
    hcsr04.EnableCapture();
    hcsr04.EnableAndStartTimer();

    vTaskDelay(pdMS_TO_TICKS(100)); // Short delay before first measurement
    while (1) {
        hcsr04.SampleAndLogDistance(xTaskGetCurrentTaskHandle());
        vTaskDelay(pdMS_TO_TICKS(200));
    }
// Troubleshooting tips:
// - Make sure TRIG is connected to ESP32 output pin, ECHO to input pin.
// - ECHO pin should NOT have a pull-up enabled in software; the sensor drives it.
// - If you see only timeout warnings, check wiring and sensor power.
// - Use a logic analyzer or oscilloscope to verify ECHO pulse if needed.
}