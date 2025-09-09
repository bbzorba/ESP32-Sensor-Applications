#include <cstdio>
#include <cstring>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"
static const char *TAG = "SDTest";

extern "C" void app_main() {
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    // For SDMMC, use default pins (CLK=14, CMD=15, D0=2, D1=4, D2=12, D3=13)
    // If using only 1-bit mode:
    // slot_config.width = 1;

    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card filesystem, error code: 0x%x", ret);
        return;
    }
    ESP_LOGI(TAG, "SD card mounted");
    ESP_LOGI(TAG, "Name: %s", card->cid.name);
    uint64_t card_size = (uint64_t)card->csd.capacity * card->csd.sector_size;
    ESP_LOGI(TAG, "Size: %lluMB", card_size / (1024 * 1024));
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    ESP_LOGI(TAG, "SD card unmounted");
}
