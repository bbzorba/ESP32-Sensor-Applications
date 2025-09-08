#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "lwip/apps/sntp.h"
#include "driver/gpio.h"
#include "esp_http_server.h" // For the basic web server

// --- Configuration Defines ---
#define WIFI_SSID      "YOUR_WIFI_SSID"    // Replace with your Wi-Fi SSID
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD" // Replace with your Wi-Fi password
#define LED_GPIO       GPIO_NUM_2         // Built-in LED on ESP32 development boards (often GPIO2)

// --- Global Variables ---
static const char *TAG = "ESP32_APP";
static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0; // Event bit for Wi-Fi connected

// --- Function Prototypes ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void initialise_wifi(void);
static void obtain_time(void);
static void time_sync_notification_cb(struct timeval *tv);
static void led_blink_task(void *pvParameters);
static esp_err_t http_server_handler(httpd_req_t *req);
static void start_web_server(void);

// --- Wi-Fi Event Handler ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected, retrying...");
        esp_wifi_connect(); // Try to reconnect
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// --- Wi-Fi Initialization ---
static void initialise_wifi(void) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization complete. Connecting to %s...", WIFI_SSID);
}

// --- NTP Time Synchronization Callback ---
static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Time synchronized: %lld us", (long long)tv->tv_sec);
    // Set timezone to your desired timezone (e.g., EST5EDT, PST8PDT, CET-1CEST-2, GMT0)
    // For example, for Central European Time (CET), which is UTC+1, and CEST (UTC+2) during daylight saving:
    setenv("TZ", "CET-1CEST-2,M3.5.0/2,M10.5.0/3", 1);
    tzset();
}

// --- Obtain Time from NTP Server ---
static void obtain_time(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org"); // Use a common NTP server pool
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();

    // Wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (retry == retry_count) {
        ESP_LOGE(TAG, "Failed to synchronize time after multiple retries.");
    } else {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "Current time after sync: %s", strftime_buf);
    }
}

// --- LED Blink Task ---
static void led_blink_task(void *pvParameters) {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_GPIO, 0); // LED ON (assuming active low for common dev boards)
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(LED_GPIO, 1); // LED OFF
        vTaskDelay(900 / portTICK_PERIOD_MS); // Total 1 second cycle
    }
}

// --- HTTP Server Handler ---
static esp_err_t http_server_handler(httpd_req_t *req) {
    time_t now;
    struct tm timeinfo;
    char time_buf[64];
    char html_response[256];

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_buf, sizeof(time_buf), "%A, %B %d, %Y %H:%M:%S", &timeinfo);

    // Prepare the HTML response
    snprintf(html_response, sizeof(html_response),
             "<!DOCTYPE html>"
             "<html>"
             "<head><title>ESP32 Time Server</title></head>"
             "<body>"
             "<h1>Welcome to ESP32!</h1>"
             "<p>Current Time: <strong>%s</strong></p>"
             "<p>Wi-Fi Status: <strong>Connected</strong></p>"
             "<p>Device is up and running!</p>"
             "</body>"
             "</html>",
             time_buf);

    httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// --- Start Web Server ---
static void start_web_server(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting web server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = http_server_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);
    } else {
        ESP_LOGE(TAG, "Error starting web server!");
    }
}

// --- Main Application ---
void app_main(void) {
    // Initialize NVS (Non-Volatile Storage) - required for Wi-Fi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP32 application started.");

    // 1. Initialize Wi-Fi and connect
    initialise_wifi();

    // Wait until Wi-Fi is connected
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi connected successfully!");

    // 2. Obtain time from NTP server
    obtain_time();

    // 3. Start LED blink task
    xTaskCreate(&led_blink_task, "led_blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

    // 4. Start a basic web server
    start_web_server();

    // Main loop (optional, can be empty if tasks handle everything)
    while (1) {
        // You can add other periodic tasks or checks here
        vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 5 seconds
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "Current time (from main loop): %s", strftime_buf);
    }
}
