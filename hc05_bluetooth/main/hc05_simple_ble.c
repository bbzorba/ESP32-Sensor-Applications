#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define RXD2 16
#define TXD2 17
#define UART_PORT_NUM UART_NUM_2

void app_main(void) {
    const uart_config_t uart2_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_PORT_NUM, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_PORT_NUM, &uart2_config);
    uart_set_pin(UART_PORT_NUM, TXD2, RXD2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uint8_t data[128];
    printf("HC-05 UART ready. Waiting for Bluetooth data...\n");
    while (1) {
        int rx_bytes = uart_read_bytes(UART_PORT_NUM, data, sizeof(data) - 1, pdMS_TO_TICKS(100));
        if (rx_bytes > 0) {
            data[rx_bytes] = 0;
            printf("RX from HC-05: %s\n", data);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}