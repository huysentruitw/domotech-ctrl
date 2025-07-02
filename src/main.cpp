#include "driver/gpio.h"
#include "esp_rom_sys.h"

#define LED_GPIO GPIO_NUM_2

void delay_ms(uint32_t ms) {
    esp_rom_delay_us(ms * 1000); // ROM-based delay, not very accurate but simple
}

void app_main() {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_GPIO, 1); // LED on
        delay_ms(500);

        gpio_set_level(LED_GPIO, 0); // LED off
        delay_ms(500);
    }
}
