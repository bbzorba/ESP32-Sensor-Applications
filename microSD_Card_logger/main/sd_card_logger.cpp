#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

using namespace std;

#define MOUNT_POINT "/sdcard"

class SDLogger {
public:
    SDLogger() : card(nullptr) {}
    bool mount() {
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 3,
            .allocation_unit_size = 16 * 1024
        };
        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        host.slot = SPI2_HOST;
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = 23,
            .miso_io_num = 19,
            .sclk_io_num = 18,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4000
        };
        esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize SPI bus");
            return false;
        }
        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.gpio_cs = GPIO_NUM_2;
        slot_config.host_id = SPI2_HOST;
        ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to mount SD card filesystem");
            return false;
        }
        ESP_LOGI(TAG, "SD card mounted");
        return true;
    }
    void unmount() {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
        ESP_LOGI(TAG, "SD card unmounted");
        spi_bus_free(SPI2_HOST);
    }
    void log_entry() {
        string log_path = string(MOUNT_POINT) + "/log.txt";
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now_c));
        ofstream log_file(log_path, ios::app);
        if (log_file.is_open()) {
            log_file << buf << " - Logging entry from ESP32" << endl;
            log_file.flush();
            log_file.close();
            ESP_LOGI(TAG, "Logged: %s", buf);
        } else {
            ESP_LOGE(TAG, "Failed to open log.txt for writing");
        }
    }
private:
    sdmmc_card_t *card;
    const char *TAG = "SDLogger";
};

extern "C" void app_main() {
    SDLogger logger;
    if (logger.mount()) {
        while (true) {
            logger.log_entry();
            this_thread::sleep_for(chrono::seconds(1));
        }
        logger.unmount();
    }
}
