#include "app.h"
#include "button.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

button_t button;


button_config_t button_config = {.pin_mask = GPIO_PIN_0,
                                 .port = GPIOB,
                                 .active_low = true,
                                 .debounce_ms = 20};

void app_init(void) {
  button_status_t button_status = button_init(&button, &button_config);

  if (button_status != BUTTON_OK) {
    // Обробка помилки ініціалізації кнопки
    printf("Button initialization failed with status: %d\n", button_status);
  } else {
    printf("Button initialized successfully.\n");
  }
}

void app_run(void) {

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
