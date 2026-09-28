#include "app.h"
#include "adc.h"
#include "button.h"
#include "encoder.h"
#include "motor.h"
#include "tim.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

encoder_t encoder;
button_t button;
static uint32_t last_print = 0u;
#define DUTY_STEP 50 /* ‰ за один клік: 20 кліків від 0 до 100 % */
#define DUTY_MAX 1000

static int32_t duty = 0;    /* задана швидкість */
static bool running = true; /* кнопка: пауза / робота */

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

const motor_config_t motor_cfg = {
    .pwm_tim = &htim2,
    .pwm_channel = TIM_CHANNEL_1,
    .tick_tim = &htim4,
    .adc = &hadc1,
};

void app_init(void) {
  encoder_status_t status = encoder_init(&encoder, &encoder_config);
  button_status_t button_status = button_init(&button, &button_config);
  HAL_StatusTypeDef motor_status = motor_init(&motor_cfg);
  if (motor_status != HAL_OK) {
    // Обробка помилки ініціалізації двигуна
    printf("Motor initialization failed with status: %d\n", motor_status);
  } else {
    printf("Motor initialized successfully.\n");
  }
  motor_set_duty(0);
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
    duty += steps * DUTY_STEP;
    if (duty < 0)
      duty = 0;
    if (duty > DUTY_MAX)
      duty = DUTY_MAX;
    printf("duty: %ld.%ld %%\n", duty / 10, duty % 10);
  }

  button_process(&button);
  if (button_was_pressed(&button)) {
    running = !running;
    printf(running ? "run\n" : "stop\n");
  }

  motor_set_duty(running ? (uint16_t)duty : 0u);
  if (HAL_GetTick() - last_print >= 500u) {
    last_print = HAL_GetTick();
    printf("bemf %lu mV, rpm %lu\n", motor_get_bemf_mv(), motor_get_rpm());
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    motor_tick_isr();
  }
}