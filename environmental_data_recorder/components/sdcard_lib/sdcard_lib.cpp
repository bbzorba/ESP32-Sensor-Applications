#include "sdcard_lib.h"
#include <cerrno>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"

static const char *TAG = "SD_CARD_LIB";

// Implementation of the SDCard class member functions.

// Constructor
SDCard::SDCard(const char* mountPoint, int mosi, int miso, int sclk, int cs) {
    mount_point = mountPoint;
    pin_mosi = mosi;
    pin_miso = miso;
    pin_sclk = sclk;
    pin_cs = cs;
    card = nullptr;
    host_id = SPI2_HOST; // Using SPI2_HOST by default
}

// Destructor
SDCard::~SDCard() {
    unmount();
}

// Initialize the SPI bus and mount the SD card
esp_err_t SDCard::init() {
    ESP_LOGI(TAG, "Initializing SPI bus and SD card...");

    gpio_set_pull_mode(static_cast<gpio_num_t>(pin_mosi), GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(static_cast<gpio_num_t>(pin_miso), GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(static_cast<gpio_num_t>(pin_sclk), GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(static_cast<gpio_num_t>(pin_cs), GPIO_PULLUP_ONLY);

    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = pin_mosi;
    bus_cfg.miso_io_num = pin_miso;
    bus_cfg.sclk_io_num = pin_sclk;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4000;

    esp_err_t ret = spi_bus_initialize(host_id, &bus_cfg, host_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus (%s).", esp_err_to_name(ret));
        return ret;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = host_id;
    host.max_freq_khz = SDMMC_FREQ_PROBING;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = static_cast<gpio_num_t>(pin_cs);
    slot_config.host_id = (spi_host_device_t)host.slot;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;
    mount_config.allocation_unit_size = 0;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                           "If you want to format the card, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                           "Make sure there is an SD card in the slot and try again.", esp_err_to_name(ret));
        }
        spi_bus_free(host_id);
        return ret;
    }

    ESP_LOGI(TAG, "SD card mounted successfully at %s", mount_point);
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

// Unmount the SD card and free resources
void SDCard::unmount() {
    if (card != nullptr) {
        esp_vfs_fat_sdcard_unmount(mount_point, card);
        ESP_LOGI(TAG, "Card unmounted");
        card = nullptr;
    }
    spi_bus_free(host_id);
}

// Function to write a simple file
void SDCard::writeFile(const char *path, const char *data) {
    if (!card) {
        ESP_LOGE(TAG, "SD card is not mounted. Cannot write file.");
        return;
    }
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);

    ESP_LOGI(TAG, "Writing file: %s", full_path);
    FILE *f = fopen(full_path, "a"); // Try append mode first
    if (f == NULL) {
        ESP_LOGW(TAG, "Append mode failed, trying write mode (errno=%d: %s)", errno, strerror(errno));
        f = fopen(full_path, "w");
    }
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing (errno=%d: %s)", errno, strerror(errno));
        return;
    }
    fprintf(f, "%s", data);
    fflush(f);
    fclose(f);
    ESP_LOGI(TAG, "File written successfully");
}

// Function to read a file
void SDCard::readFile(const char *path) {
    if (!card) {
        ESP_LOGE(TAG, "SD card is not mounted. Cannot read file.");
        return;
    }
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);

    ESP_LOGI(TAG, "Reading file: %s", full_path);
    FILE *f = fopen(full_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[128];
    while (fgets(line, sizeof(line), f) != NULL) {
        ESP_LOGI(TAG, "Read: %s", line);
    }
    fclose(f);
}

// Function to create a new directory
void SDCard::createDirectory(const char *path) {
    if (!card) {
        ESP_LOGE(TAG, "SD card is not mounted. Cannot create directory.");
        return;
    }
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);

    ESP_LOGI(TAG, "Creating directory: %s", full_path);
    int res = mkdir(full_path, 0777);
    if (res != 0) {
        ESP_LOGE(TAG, "Failed to create directory %s (errno=%d: %s)", full_path, errno, strerror(errno));
    } else {
        ESP_LOGI(TAG, "Directory created successfully: %s", full_path);
    }
}

// Check if a directory exists
bool SDCard::directoryExists(const char *path) {
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);
    struct stat st;
    if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    return false;
}

// Function to delete a file
void SDCard::deleteFile(const char *path) {
    if (!card) {
        ESP_LOGE(TAG, "SD card is not mounted. Cannot delete file.");
        return;
    }
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);

    ESP_LOGI(TAG, "Deleting file: %s", full_path);
    if (unlink(full_path) != 0) {
        ESP_LOGE(TAG, "Failed to delete file");
    } else {
        ESP_LOGI(TAG, "File deleted successfully");
    }
}

// Function to delete a directory
void SDCard::deleteDirectory(const char *path) {
    if (!card) {
        ESP_LOGE(TAG, "SD card is not mounted. Cannot delete directory.");
        return;
    }
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);

    ESP_LOGI(TAG, "Deleting directory: %s", full_path);
    if (rmdir(full_path) != 0) {
        ESP_LOGE(TAG, "Failed to delete directory");
    } else {
        ESP_LOGI(TAG, "Directory deleted successfully");
    }
}
