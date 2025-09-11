#ifndef SDCARD_LIB_H
#define SDCARD_LIB_H

#include "esp_err.h"
#include "driver/sdmmc_host.h"
#include <stdio.h> // Included for FILE type
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"

// This is the public C++ header for the SDCard class.
// It declares the class and its public member functions.
class SDCard {
private:
    const char* mount_point;
    sdmmc_card_t* card;
    spi_host_device_t host_id;
    int pin_mosi, pin_miso, pin_sclk, pin_cs;

public:
    // Constructor to set up the pins and mount point
    SDCard(const char* mountPoint, int mosi, int miso, int sclk, int cs);

    // Destructor to ensure resources are freed
    ~SDCard();

    // Initialize the SPI bus and mount the SD card
    esp_err_t init();

    // Unmount the SD card and free resources
    void unmount();
    
    // Function to write a simple file
    void writeFile(const char *path, const char *data);

    // Function to read a file
    void readFile(const char *path);
    
    // Function to create a new directory
    void createDirectory(const char *path);

    // Function to delete a file
    void deleteFile(const char *path);

    // Function to delete a directory
    void deleteDirectory(const char *path);

    // Check if a directory exists
    bool directoryExists(const char *path);
};

#endif // SDCARD_LIB_H
