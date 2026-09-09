#pragma once

#include "driver/gpio.h"

#define APP_BUTTON_GPIO GPIO_NUM_4
#define APP_LED_GPIO GPIO_NUM_5

constexpr std::uint32_t APP_BUTTON_DEBOUNCE_MS = 50;
constexpr std::uint32_t APP_LONG_PRESS_MS = 1000;

constexpr std::uint32_t APP_LED_BLINK_PERIOD_MS = 1000;
