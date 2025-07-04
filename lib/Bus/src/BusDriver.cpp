#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_rom_sys.h"
#include "BusDriver.h"

#define READ_TIMEOUT_MS   16

#define UART_PORT         UART_NUM_2
#define TXD_PIN           GPIO_NUM_17
#define RXD_PIN           GPIO_NUM_16
#define DE_PIN            GPIO_NUM_4     // RS485 DE/RE control pin

#undef AUTO_DE

void BusDriver::Init()
{
    // Configure UART2 for RS485 (no parity, 1 stop bit, 8 data bits)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const uart_config_t uart_config =
    {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
    };
    #pragma GCC diagnostic pop

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

#ifdef AUTO_DE
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, DE_PIN, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));
#else
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_UART));

    gpio_reset_pin(DE_PIN);
    gpio_set_direction(DE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DE_PIN, 0);
#endif
    ESP_ERROR_CHECK(uart_set_rx_timeout(UART_PORT, 1));
}

void BusDriver::WriteBytes(const uint8_t* data, const uint16_t len)
{
#ifndef AUTO_DE    
    gpio_set_level(DE_PIN, 1);
#endif

    // Need to send in a loop, otherwise PICs can't process the incoming bytes fast enough
    for (int i = 0; i < len; i++)
    {
        if (i > 0) esp_rom_delay_us(104);
        uart_write_bytes(UART_PORT, &data[i], 1);
        uart_wait_tx_done(UART_PORT, pdMS_TO_TICKS(20));
    }

#ifndef AUTO_DE
    gpio_set_level(DE_PIN, 0);
#endif
}

bool BusDriver::ReadBytes(uint8_t* data, const uint16_t len)
{
    return uart_read_bytes(UART_PORT, data, len, pdMS_TO_TICKS(READ_TIMEOUT_MS)) == len;
}

void BusDriver::FlushInput()
{
    uart_flush_input(UART_PORT);
}
