#include <string>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h> // for rmdir
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
using namespace std;

class SDCard {
public:
  SDCard(const string& mount_point = "/sdcard") : mount_point_(mount_point), card_(nullptr), mounted_(false) {}

  bool mount() {
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024
    };
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = 23;
    bus_cfg.miso_io_num = 19;
    bus_cfg.sclk_io_num = 18;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4000;
    esp_err_t ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
      ESP_LOGE(TAG, "Failed to initialize SPI bus.");
      return false;
    }
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)2;
    slot_config.host_id = (spi_host_device_t)host.slot;
    ret = esp_vfs_fat_sdspi_mount(mount_point_.c_str(), &host, &slot_config, &mount_config, &card_);
    if (ret != ESP_OK) {
      if (ret == 0x109) {
        ESP_LOGE(TAG, "SD card mount failed: Card not detected or not compatible (0x109). Check wiring, card type, and pull-ups.");
      } else {
        ESP_LOGE(TAG, "Failed to mount SD card filesystem. Error: 0x%x", ret);
      }
      spi_bus_free((spi_host_device_t)host.slot);
      return false;
    }
    mounted_ = true;
    ESP_LOGI(TAG, "SD card mounted.");
    ESP_LOGI(TAG, "Name: %s", card_->cid.name);
    uint64_t card_size = (uint64_t)card_->csd.capacity * card_->csd.sector_size;
    ESP_LOGI(TAG, "Size: %lluMB", card_size / (1024 * 1024));
    return true;
  }

  void unmount() {
    if (mounted_) {
      esp_vfs_fat_sdcard_unmount(mount_point_.c_str(), card_);
      spi_bus_free(SPI2_HOST);
      ESP_LOGI(TAG, "SD card unmounted.");
      mounted_ = false;
    }
  }

  bool write_file(const string& path, const string& data) {
    string full_path = mount_point_ + path;
    FILE* f = fopen(full_path.c_str(), "w");
    if (!f) return false;
    fwrite(data.c_str(), 1, data.size(), f);
    fclose(f);
    return true;
  }

  bool read_file(const string& path, string& out) {
    string full_path = mount_point_ + path;
    FILE* f = fopen(full_path.c_str(), "r");
    if (!f) return false;
    char buf[128];
    out.clear();
    while (fgets(buf, sizeof(buf), f)) out += buf;
    fclose(f);
    return true;
  }

  bool create_dir(const string& path) {
    string full_path = mount_point_ + path;
    return mkdir(full_path.c_str(), 0777) == 0;
  }

  bool delete_dir(const string& path) {
    string full_path = mount_point_ + path;
    return rmdir(full_path.c_str()) == 0;
  }

  ~SDCard() { unmount(); }

private:
  string mount_point_;
  sdmmc_card_t* card_;
  bool mounted_;
  static constexpr const char* TAG = "SDCard";
};


extern "C" void app_main() {
  SDCard sd;
  if (sd.mount()) {
    sd.write_file("/test.txt", "Hello SD Card!\n");
    string content;
    if (sd.read_file("/test.txt", content)) {
      ESP_LOGI("SDCard", "Read: %s", content.c_str());
    }
    sd.create_dir("/mydir");
    sd.delete_dir("/mydir");
    sd.unmount();
  }
}