#include "button.h"
#include <stdio.h>

#define BUTTON_DEBOUNCE_MS 50U

struct button_s {
  button_config_t config;
  ButtonStatus status;
  bool pressed;                   // debounced physical level, released<->pressed
  bool press_event;               // one-shot: latched on press, consumed by button_read()
  volatile bool edge_pending;     // set by the ISR, cleared once debounced
  volatile uint32_t last_edge_tick;
};

static struct button_s self;

// GPIO pin, EXTI and NVIC setup for the button is owned by CubeMX-generated
// MX_GPIO_Init() (gpio.c), which runs before app_init() in main.c.
button_handler_t button_init(button_config_t config) {
  self.config = config;
  self.status = BUTTON_ERROR;
  self.pressed = false;
  self.press_event = false;
  self.edge_pending = false;
  self.last_edge_tick = 0;
  return &self;
}

// Returns true exactly once per confirmed press, then resets until the next one.
bool button_read(button_handler_t button) {
  if (button == NULL || !button->press_event) {
    return false;
  }
  button->press_event = false;
  return true;
}

void button_process(button_handler_t button) {
  if (button == NULL || !button->edge_pending) {
    return;
  }

  if ((HAL_GetTick() - button->last_edge_tick) < BUTTON_DEBOUNCE_MS) {
    return; // line may still be bouncing, wait it out
  }
  button->edge_pending = false;

  GPIO_PinState level = HAL_GPIO_ReadPin(button->config.port, button->config.pin);
  bool physically_pressed = button->config.active_low ? (level == GPIO_PIN_RESET)
                                                        : (level == GPIO_PIN_SET);

  if (physically_pressed != button->pressed) {
    button->pressed = physically_pressed;
    button->status = BUTTON_OK;
    if (physically_pressed) {
      button->press_event = true;
      printf("Button Pressed\n");
    } else {
      printf("Button Released\n");
    }
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == self.config.pin) {
    self.last_edge_tick = HAL_GetTick();
    self.edge_pending = true;

    printf("detected press at %lu ms\n", self.last_edge_tick);
  }
}
