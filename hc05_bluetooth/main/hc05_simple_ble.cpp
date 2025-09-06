#include <cstdio>
#include <cstring>
#include <array>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

class HC05 {
public:
    static constexpr int RXD = 16;
    static constexpr int TXD = 17;
    static constexpr uart_port_t UART_PORT = UART_NUM_2;
    static constexpr size_t BUF_SIZE = 128;
    int baud_rate;
    std::array<uint8_t, BUF_SIZE> buffer;

    HC05(int baud = 9600) : baud_rate(baud) {
        uart_config_t uart2_config = {
            .baud_rate = baud_rate,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_APB,
        };
        uart_driver_install(UART_PORT, 1024, 0, 0, nullptr, 0);
        uart_param_config(UART_PORT, &uart2_config);
        uart_set_pin(UART_PORT, TXD, RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        std::printf("HC-05 UART ready. Waiting for Bluetooth data...\n");
    }

    void receive_loop() {
        while (true) {
            int rx_bytes = uart_read_bytes(UART_PORT, buffer.data(), buffer.size() - 1, pdMS_TO_TICKS(100));
            if (rx_bytes > 0) {
                buffer[rx_bytes] = 0;
                std::printf("RX from HC-05: %s\n", buffer.data());
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
};

extern "C" void app_main(void) {
    static HC05 hc05;
    hc05.receive_loop();
}
