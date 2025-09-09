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

static const char *TAG = "SD_CARD";

// Function to write a simple file
void write_file(const char *path, const char *data) {
    ESP_LOGI(TAG, "Writing file: %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "File written successfully");
}

// Function to read a file
void read_file(const char *path) {
    ESP_LOGI(TAG, "Reading file: %s", path);
    FILE *f = fopen(path, "r");
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
void create_directory(const char *path) {
    ESP_LOGI(TAG, "Creating directory: %s", path);
    if (mkdir(path, 0777) != 0) {
        ESP_LOGE(TAG, "Failed to create directory");
    } else {
        ESP_LOGI(TAG, "Directory created successfully");
    }
}

// Function to delete a file
void delete_file(const char *path) {
    ESP_LOGI(TAG, "Deleting file: %s", path);
    if (unlink(path) != 0) {
        ESP_LOGE(TAG, "Failed to delete file");
    } else {
        ESP_LOGI(TAG, "File deleted successfully");
    }
}

// Function to delete a directory
void delete_directory(const char *path) {
    ESP_LOGI(TAG, "Deleting directory: %s", path);
    if (rmdir(path) != 0) {
        ESP_LOGE(TAG, "Failed to delete directory");
    } else {
        ESP_LOGI(TAG, "Directory deleted successfully");
    }
}

void app_main(void)
{
    esp_err_t ret;

    // Define the mount point
    const char mount_point[] = "/sdcard";

    ESP_LOGI(TAG, "Initializing SPI bus and SD card...");
    
    // Set up SPI bus configuration
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = 23, // Your MOSI pin
        .miso_io_num = 19, // Your MISO pin
        .sclk_io_num = 18, // Your SCLK pin
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    
    // Explicitly set pull-ups on the SPI bus lines
    gpio_set_pull_mode(23, GPIO_PULLUP_ONLY); // MOSI
    gpio_set_pull_mode(19, GPIO_PULLUP_ONLY); // MISO
    gpio_set_pull_mode(18, GPIO_PULLUP_ONLY); // SCLK
    
    // Initialize the SPI bus
    ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus (%s).", esp_err_to_name(ret));
        return;
    }

    // Set up host configuration for the SD card, specifying the SPI host
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    host.max_freq_khz = SDMMC_FREQ_PROBING; 

    // Set up device configuration, specifically for the Chip Select (CS) pin
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = 2; // Your Chip Select pin
    slot_config.host_id = host.slot;

    // Explicitly set a pull-up on the CS pin as well
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY); // CS
    
    // Configure the FATFS volume mount
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Card handle
    sdmmc_card_t *card;

    // Mount the SD card
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want to format the card, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure there is an SD card in the slot and try again.", esp_err_to_name(ret));
        }
        spi_bus_free(host.slot);
        return;
    }

    ESP_LOGI(TAG, "SD card mounted successfully at %s", mount_point);
    sdmmc_card_print_info(stdout, card);

    // --- File System Operations ---
    
    // Create a directory
    create_directory("/sdcard/test_dir");

    // Write a file in the new directory
    write_file("/sdcard/test_dir/hello.txt", "Hello from ESP-IDF!");

    // Read the file
    read_file("/sdcard/test_dir/hello.txt");
    
    // Delete the file
    delete_file("/sdcard/test_dir/hello.txt");

    // Delete the directory
    delete_directory("/sdcard/test_dir");

    // Unmount the card and free resources
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
}
