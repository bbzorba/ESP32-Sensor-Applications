#include "qwiicrf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <vector>

static const char *TAG = "QwiicRF";

// SparkFun QwiicRF command bytes (from Arduino library)
static constexpr uint8_t CMD_GET_STATUS           = 0x01;
static constexpr uint8_t CMD_SEND                 = 0x02;
static constexpr uint8_t CMD_SEND_RELIABLE        = 0x03;
static constexpr uint8_t CMD_SET_RELIABLE_TIMEOUT = 0x04;
static constexpr uint8_t CMD_GET_PAYLOAD          = 0x05;
static constexpr uint8_t CMD_SET_SPREAD_FACTOR    = 0x06;
static constexpr uint8_t CMD_SET_SYNC_WORD        = 0x07;
static constexpr uint8_t CMD_SET_RF_ADDRESS       = 0x08;
static constexpr uint8_t CMD_GET_RF_ADDRESS       = 0x09;
static constexpr uint8_t CMD_GET_PACKET_RSSI      = 0x0A;
static constexpr uint8_t CMD_GET_PAYLOAD_SIZE     = 0x0B;
static constexpr uint8_t CMD_GET_PACKET_SENDER    = 0x0C;
static constexpr uint8_t CMD_GET_PACKET_RECIPIENT = 0x0D;
static constexpr uint8_t CMD_GET_PACKET_SNR       = 0x0E;
static constexpr uint8_t CMD_GET_PACKET_ID        = 0x0F;
static constexpr uint8_t CMD_SET_TX_POWER         = 0x10;
static constexpr uint8_t CMD_GET_SYNC_WORD        = 0x11;
static constexpr uint8_t CMD_SET_PAIRED_ADDRESS   = 0x12;
static constexpr uint8_t CMD_GET_PAIRED_ADDRESS   = 0x13;
// Note: SparkFun header shows overlaps (e.g., 0x20 used for both set I2C and send paired).
// We'll use 0x20 for send paired per header comments. We are not changing I2C addr here.
static constexpr uint8_t CMD_SEND_PAIRED          = 0x20;

QwiicRF::QwiicRF(i2c_port_t i2c_port, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz)
    : _i2c_port(i2c_port),
      _sda_pin(sda_pin),
      _scl_pin(scl_pin),
      _clk_speed_hz(clk_speed_hz),
      _device_address(DEFAULT_ADDR) {
}

esp_err_t QwiicRF::init() {
    // configure the I2C master
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = _sda_pin;
    conf.scl_io_num = _scl_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = _clk_speed_hz;
    esp_err_t err = i2c_param_config(_i2c_port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %d", err);
        return err;
    }

    err = i2c_driver_install(_i2c_port, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %d", err);
        return err;
    }

    // Optionally: check device responsiveness (non-fatal)
    size_t avail = 0;
    (void)packetAvailable(&avail, pdMS_TO_TICKS(20));

    ESP_LOGI(TAG, "QwiicRF init done");
    return ESP_OK;
}

esp_err_t QwiicRF::i2cWrite(const uint8_t *data, size_t len, TickType_t ticks_to_wait) {
    // Use I2C master write
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // device address + write bit
    i2c_master_write_byte(cmd, (_device_address << 1) | I2C_MASTER_WRITE, true);
    // ESP-IDF v4.2 API expects non-const pointer; data is not modified by driver
    i2c_master_write(cmd, const_cast<uint8_t*>(data), len, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(_i2c_port, cmd, ticks_to_wait);
    i2c_cmd_link_delete(cmd);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2cWrite error: %d", err);
    }
    return err;
}

esp_err_t QwiicRF::i2cRead(uint8_t *data, size_t len, TickType_t ticks_to_wait) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_device_address << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    // last byte
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(_i2c_port, cmd, ticks_to_wait);
    i2c_cmd_link_delete(cmd);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2cRead error: %d", err);
    }
    return err;
}

esp_err_t QwiicRF::sendPacket(const uint8_t *data, size_t len, TickType_t ticks_to_wait) {
    if (len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    // Send to the paired address
    std::vector<uint8_t> buf;
    buf.reserve(1 + len);
    buf.push_back(CMD_SEND_PAIRED);
    buf.insert(buf.end(), data, data + len);
    return i2cWrite(buf.data(), buf.size(), ticks_to_wait);
}

esp_err_t QwiicRF::sendPacketTo(uint8_t rf_addr, const uint8_t *data, size_t len, TickType_t ticks_to_wait) {
    if (len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    // Build buffer: [CMD_SEND][rf_addr][payload...]
    std::vector<uint8_t> buf;
    buf.reserve(2 + len);
    buf.push_back(CMD_SEND);
    buf.push_back(rf_addr);
    buf.insert(buf.end(), data, data + len);
    return i2cWrite(buf.data(), buf.size(), ticks_to_wait);
}

esp_err_t QwiicRF::packetAvailable(size_t *out_len, TickType_t ticks_to_wait) {
    if (out_len == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    // Ask for payload size, read back one byte with repeated-start
    uint8_t len = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_device_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, CMD_GET_PAYLOAD_SIZE, true);
    i2c_master_start(cmd); // repeated start
    i2c_master_write_byte(cmd, (_device_address << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &len, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(_i2c_port, cmd, ticks_to_wait);
    i2c_cmd_link_delete(cmd);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "packetAvailable: cmd_begin failed: %d", err);
        *out_len = 0;
        return err;
    }
    *out_len = len;
    return ESP_OK;
}

esp_err_t QwiicRF::readPacket(uint8_t *buffer, size_t buffer_len, size_t *out_len, TickType_t ticks_to_wait) {
    if (buffer == nullptr || out_len == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t avail = 0;
    esp_err_t err = packetAvailable(&avail, ticks_to_wait);
    if (err != ESP_OK) {
        return err;
    }
    if (avail == 0) {
        *out_len = 0;
        return ESP_OK;
    }
    if (avail > buffer_len) {
        // too big, limit
        avail = buffer_len;
    }

    // Read using write (command) then repeated-start read
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, (_device_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd2, CMD_GET_PAYLOAD, true);
    i2c_master_start(cmd2); // repeated start
    i2c_master_write_byte(cmd2, (_device_address << 1) | I2C_MASTER_READ, true);
    if (avail > 1) {
        i2c_master_read(cmd2, buffer, avail - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd2, buffer + avail - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd2);
    err = i2c_master_cmd_begin(_i2c_port, cmd2, ticks_to_wait);
    i2c_cmd_link_delete(cmd2);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "readPacket: cmd_begin failed: %d", err);
        return err;
    }
    *out_len = avail;
    return ESP_OK;
}

esp_err_t QwiicRF::setRFAddress(uint8_t addr, TickType_t ticks_to_wait) {
    uint8_t payload[2] = { CMD_SET_RF_ADDRESS, addr };
    return i2cWrite(payload, sizeof(payload), ticks_to_wait);
}

esp_err_t QwiicRF::setPairedAddress(uint8_t addr, TickType_t ticks_to_wait) {
    uint8_t payload[2] = { CMD_SET_PAIRED_ADDRESS, addr };
    return i2cWrite(payload, sizeof(payload), ticks_to_wait);
}