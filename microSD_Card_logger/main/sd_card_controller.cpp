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

class SDCard {
private:
    const char* mount_point;
    sdmmc_card_t* card;
    spi_host_device_t host_id;
    int pin_mosi, pin_miso, pin_sclk, pin_cs;

public:
    // Constructor to set up the pins and mount point
    SDCard(const char* mountPoint, int mosi, int miso, int sclk, int cs) {
        mount_point = mountPoint;
        pin_mosi = mosi;
        pin_miso = miso;
        pin_sclk = sclk;
        pin_cs = cs;
        card = nullptr;
        host_id = SPI2_HOST; // Using SPI2_HOST by default
    }

    // Initialize the SPI bus and mount the SD card
    esp_err_t init() {
        ESP_LOGI(TAG, "Initializing SPI bus and SD card...");

        // Explicitly set pull-ups on the SPI bus lines, casting int to gpio_num_t
        gpio_set_pull_mode(static_cast<gpio_num_t>(pin_mosi), GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(static_cast<gpio_num_t>(pin_miso), GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(static_cast<gpio_num_t>(pin_sclk), GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(static_cast<gpio_num_t>(pin_cs), GPIO_PULLUP_ONLY);
        
        // Set up SPI bus configuration
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = pin_mosi,
            .miso_io_num = pin_miso,
            .sclk_io_num = pin_sclk,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .data4_io_num = -1,
            .data5_io_num = -1,
            .data6_io_num = -1,
            .data7_io_num = -1,
            .data_io_default_level = 0,
            .max_transfer_sz = 4000,
            .flags = 0,
            .intr_flags = 0,
        };
        
        // Initialize the SPI bus
        esp_err_t ret = spi_bus_initialize(host_id, &bus_cfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize SPI bus (%s).", esp_err_to_name(ret));
            return ret;
        }

        // Set up host configuration for the SD card, specifying the SPI host
        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        host.slot = host_id;
        host.max_freq_khz = SDMMC_FREQ_PROBING; 

        // Set up device configuration, specifically for the Chip Select (CS) pin
        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.gpio_cs = static_cast<gpio_num_t>(pin_cs);
        slot_config.host_id = (spi_host_device_t)host.slot;

        // Configure the FATFS volume mount
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024,
            .disk_status_check_enable = 0,
            .use_one_fat = 0
        };

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
            spi_bus_free(host_id);
            return ret;
        }

        ESP_LOGI(TAG, "SD card mounted successfully at %s", mount_point);
        sdmmc_card_print_info(stdout, card);
        return ESP_OK;
    }

    // Unmount the SD card and free resources
    void unmount() {
        if (card != nullptr) {
            esp_vfs_fat_sdcard_unmount(mount_point, card);
            ESP_LOGI(TAG, "Card unmounted");
            card = nullptr;
        }
        spi_bus_free(host_id);
    }
    
    // Function to write a simple file
    void writeFile(const char *path, const char *data) {
        if (!card) {
            ESP_LOGE(TAG, "SD card is not mounted. Cannot write file.");
            return;
        }
        char full_path[128];
        snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);

        ESP_LOGI(TAG, "Writing file: %s", full_path);
        FILE *f = fopen(full_path, "w");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for writing");
            return;
        }
        fprintf(f, "%s", data);
        fclose(f);
        ESP_LOGI(TAG, "File written successfully");
    }

    // Function to read a file
    void readFile(const char *path) {
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
    void createDirectory(const char *path) {
        if (!card) {
            ESP_LOGE(TAG, "SD card is not mounted. Cannot create directory.");
            return;
        }
        char full_path[128];
        snprintf(full_path, sizeof(full_path), "%s/%s", mount_point, path);

        ESP_LOGI(TAG, "Creating directory: %s", full_path);
        if (mkdir(full_path, 0777) != 0) {
            ESP_LOGE(TAG, "Failed to create directory");
        } else {
            ESP_LOGI(TAG, "Directory created successfully");
        }
    }

    // Function to delete a file
    void deleteFile(const char *path) {
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
    void deleteDirectory(const char *path) {
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
};

extern "C" void app_main(void)
{
    // Create an instance of the SDCard class with your specific pin configuration
    SDCard mySDCard("/sdcard", 23, 19, 18, 2);

    // Initialize and mount the card
    if (mySDCard.init() != ESP_OK) {
        ESP_LOGE(TAG, "SD card initialization failed. Cannot proceed with file operations.");
        return;
    }

    // --- File System Operations ---
    
    // Create a directory
    mySDCard.createDirectory("test_dir");

    // Write a file in the new directory
    mySDCard.writeFile("test_dir/hello.txt", "Hello from ESP-IDF!");

    // Read the file
    mySDCard.readFile("test_dir/hello.txt");
    
    // Delete the file
    mySDCard.deleteFile("test_dir/hello.txt");

    // Delete the directory
    mySDCard.deleteDirectory("test_dir");

    // Create another directory
    mySDCard.createDirectory("new_dir");

    // Write another file in this directory
    mySDCard.writeFile("new_dir/esp32.txt", "esp32_data!\n");

    // Unmount the card and free resources
    mySDCard.unmount();
}
