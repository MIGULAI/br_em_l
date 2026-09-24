#include "app.h"
#include "button.h"
#include "encoder.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

encoder_t encoder;
button_t button;

encoder_config_t encoder_config = {
    .tim_instance = TIM3,
    .ch_a_config = {.pin = GPIO_PIN_6,
                    .port = GPIOA,
                    .alternate = GPIO_AF2_TIM3}, // CLK
    .ch_b_config = {.pin = GPIO_PIN_7,
                    .port = GPIOA,
                    .alternate = GPIO_AF2_TIM3}, // DT
    .counts_per_detent = 4U,
    .input_filter = 15U,
    .invert_direction = false};
button_config_t button_config = {.pin_mask = GPIO_PIN_0,
                                 .port = GPIOB,
                                 .active_low = false,
                                 .debounce_ms = 50};

void app_init(void) {
  encoder_status_t status = encoder_init(&encoder, &encoder_config);
  button_status_t button_status = button_init(&button, &button_config);
  if (status != ENCODER_OK) {
    // Обробка помилки ініціалізації енкодера
    printf("Encoder initialization failed with status: %d\n", status);
  } else {
    printf("Encoder initialized successfully.\n");
  }
  if (button_status != BUTTON_OK) {
    // Обробка помилки ініціалізації кнопки
    printf("Button initialization failed with status: %d\n", button_status);
  } else {
    printf("Button initialized successfully.\n");
  }
}

void app_run(void) {
  int32_t steps = encoder_get_step(&encoder);
  if (steps != 0) {
    printf("Encoder steps: %ld\n", steps);
  }
  button_process(&button);
  if (button_was_pressed(&button)) {
    printf("Button was pressed!\n");
  }
}

int _write(int file, char *ptr, int len) {
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == button.config->pin_mask) {
    button_irq_handler(&button);
  }
}