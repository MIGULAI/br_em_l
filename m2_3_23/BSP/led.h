#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

#define LED_MAX_COUNT 4U


typedef struct led_s *led_handler_t;

typedef struct {
    uint16_t pin;
    GPIO_TypeDef *port;
    bool active_high;
    uint32_t change_delay_ms;
} led_config_t;

led_handler_t led_init(led_config_t led_config);
void          led_on(led_handler_t led);
void          led_off(led_handler_t led);
void          led_toggle(led_handler_t led);
void          led_set(led_handler_t led, bool on);
bool          led_is_on(led_handler_t led);
bool          led_should_toggle(led_handler_t led);