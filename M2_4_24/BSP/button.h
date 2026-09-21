#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

typedef enum {
    BUTTON_OK = 0,
    BUTTON_ERROR = 1
} ButtonStatus;

typedef struct {
    uint8_t pin;          // GPIO pin number
    GPIO_TypeDef *port;   // GPIO port (e.g., GPIOA, GPIOB, etc.)
    bool active_low;      // true if button is active low, false if active high
} button_config_t;

typedef struct button_s *button_handler_t;

button_handler_t button_init(button_config_t config);

// Returns true exactly once per confirmed press, then false until the next one.
bool button_read(button_handler_t button);

// Call periodically from the main loop. Debounces and confirms edges raised
// by the EXTI interrupt; the interrupt itself only records that one occurred.
void button_process(button_handler_t button);

