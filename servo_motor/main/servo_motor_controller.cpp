// Simple single-file servo controller for ESP32 (ESP-IDF) in C++
// Uses LEDC PWM at 50 Hz to control a hobby servo.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

// Change this to the GPIO you wired your servo signal to
// Common choices: GPIO18, GPIO19, GPIO21, GPIO22, etc.
static constexpr gpio_num_t SERVO_PIN = GPIO_NUM_13;

class SimpleServo {
public:
	// Configure defaults: 50 Hz, 16-bit resolution, 1000-2000us pulse range
	SimpleServo(gpio_num_t pin,
				ledc_channel_t channel = LEDC_CHANNEL_0,
				ledc_timer_t timer = LEDC_TIMER_0,
				ledc_mode_t speed_mode = LEDC_LOW_SPEED_MODE,
				uint32_t freq_hz = 50,
				uint32_t duty_resolution_bits = 16,
				uint32_t min_pulse_us = 500,
				uint32_t max_pulse_us = 2500)
		: pin_(pin), channel_(channel), timer_(timer), speed_mode_(speed_mode),
		  freq_hz_(freq_hz), duty_bits_(duty_resolution_bits),
		  min_us_(min_pulse_us), max_us_(max_pulse_us) {}

	esp_err_t init() {
		// Configure LEDC timer
		ledc_timer_config_t tcfg{};
		tcfg.speed_mode = speed_mode_;
		tcfg.timer_num = timer_;
		tcfg.duty_resolution = static_cast<ledc_timer_bit_t>(duty_bits_);
		tcfg.freq_hz = freq_hz_;
		tcfg.clk_cfg = LEDC_AUTO_CLK;
		esp_err_t err = ledc_timer_config(&tcfg);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "timer config failed: %s", esp_err_to_name(err));
			return err;
		}

		// Configure LEDC channel
		ledc_channel_config_t ccfg{};
		ccfg.gpio_num = pin_;
		ccfg.speed_mode = speed_mode_;
		ccfg.channel = channel_;
		ccfg.intr_type = LEDC_INTR_DISABLE;
		ccfg.timer_sel = timer_;
		ccfg.duty = 0;          // start at 0 duty
		ccfg.hpoint = 0;
		err = ledc_channel_config(&ccfg);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "channel config failed: %s", esp_err_to_name(err));
			return err;
		}

		// Move to a safe center position on init
		return center();
	}

	// Move servo to specific angle in degrees [0..180]
	esp_err_t set_angle(float degrees) {
		if (degrees < 0.0f) degrees = 0.0f;
		if (degrees > 180.0f) degrees = 180.0f;
		const uint32_t pulse_us = angle_to_pulse_us(degrees);
		return set_pulse_width_us(pulse_us);
	}

	// Convenience methods (defined inside the class)
	esp_err_t move_servo_0() { return set_angle(0.0f); }
	esp_err_t move_servo_90_clockwise() { return set_angle(90.0f); }
	esp_err_t move_servo_180_clockwise() { return set_angle(180.0f); }
	esp_err_t center() { return set_angle(90.0f); }

private:
	static constexpr const char* TAG = "SimpleServo";

	// Convert angle to pulse width (microseconds)
	uint32_t angle_to_pulse_us(float deg) const {
		// Linear map 0..180 deg -> min_us_..max_us_
		float span = static_cast<float>(max_us_ - min_us_);
		return static_cast<uint32_t>(min_us_ + (deg / 180.0f) * span);
	}

	// Apply the pulse width by converting to LEDC duty
	esp_err_t set_pulse_width_us(uint32_t pulse_us) {
		// Clamp to configured range
		if (pulse_us < min_us_) pulse_us = min_us_;
		if (pulse_us > max_us_) pulse_us = max_us_;

		const uint32_t period_us = 1000000UL / freq_hz_;
		const uint32_t duty_max = (1UL << duty_bits_) - 1UL;
		uint32_t duty = (pulse_us * duty_max) / period_us;
		if (duty > duty_max) duty = duty_max;

		esp_err_t err = ledc_set_duty(speed_mode_, channel_, duty);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "set duty failed: %s", esp_err_to_name(err));
			return err;
		}
		err = ledc_update_duty(speed_mode_, channel_);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "update duty failed: %s", esp_err_to_name(err));
			return err;
		}
		return ESP_OK;
	}

	gpio_num_t pin_;
	ledc_channel_t channel_;
	ledc_timer_t timer_;
	ledc_mode_t speed_mode_;
	uint32_t freq_hz_;
	uint32_t duty_bits_;
	uint32_t min_us_;
	uint32_t max_us_;
};

extern "C" void app_main(void) {
	SimpleServo servo(SERVO_PIN);
	if (servo.init() != ESP_OK) {
		// If init fails, just idle
		while (true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
	}

	// Keep the main code as simple as possible: loop through a few positions
	while (true) {
		servo.move_servo_0();
		vTaskDelay(pdMS_TO_TICKS(1000));

		servo.move_servo_90_clockwise();
		vTaskDelay(pdMS_TO_TICKS(1000));

		servo.move_servo_180_clockwise();
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

