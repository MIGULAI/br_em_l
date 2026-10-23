#include "app.h"
#include "adc.h"
#include "motor.h"
#include "tim.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

#define SCHED_ON_MS 1500u  /* двигун працює 15 с */
#define SCHED_OFF_MS 4500u /* решта хвилини — стоп */
#define DUTY_FULL 1000u    /* 100 % */

typedef enum { SCHED_ON, SCHED_OFF } sched_state_t;

static sched_state_t sched_state = SCHED_ON;
static uint32_t sched_t0 = 0u; /* момент входу в поточний стан */

static void sched_enter(sched_state_t s) {
  sched_state = s;
  sched_t0 = HAL_GetTick();
  motor_set_duty(s == SCHED_ON ? DUTY_FULL : 0u);

  printf(s == SCHED_ON ? "motor ON\n" : "motor OFF\n");
}

static void sched_process(void) {
  uint32_t elapsed = HAL_GetTick() - sched_t0;
  switch (sched_state) {
  case SCHED_ON:
    if (elapsed >= SCHED_ON_MS)
      sched_enter(SCHED_OFF);
    break;
  case SCHED_OFF:
    if (elapsed >= SCHED_OFF_MS)
      sched_enter(SCHED_ON);
    break;
  }
}

const motor_config_t motor_cfg = {
    .pwm_tim = &htim2,
    .pwm_channel = TIM_CHANNEL_1,
    .tick_tim = &htim4,
    .adc = &hadc1,
};

void app_init(void) {
  HAL_StatusTypeDef motor_status = motor_init(&motor_cfg);
  if (motor_status != HAL_OK) {
    // Обробка помилки ініціалізації двигуна
    printf("Motor initialization failed with status: %d\n", motor_status);
  } else {
    printf("Motor initialized successfully.\n");
  }
  motor_set_duty(0);
  sched_enter(SCHED_ON);
}

void app_run(void) { sched_process(); }

int _write(int file, char *ptr, int len) {
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    motor_tick_isr();
  }
}