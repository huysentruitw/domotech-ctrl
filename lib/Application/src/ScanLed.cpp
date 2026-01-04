#include "ScanLed.h"

#ifndef NATIVE_BUILD
  #include "driver/gpio.h"
#else
  #define gpio_reset_pin(_)
  #define gpio_set_direction(_, __)
  #define gpio_set_level(_, __)
#endif

#define LED_GPIO GPIO_NUM_2

ScanLed::ScanLed() noexcept
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

void ScanLed::Toggle() const noexcept
{
    m_state = !m_state;
    gpio_set_level(LED_GPIO, m_state);
}
