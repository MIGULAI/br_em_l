#pragma once

#include "main.h" /* HAL: GPIO_TypeDef, GPIO_PIN_x, EXTI */
#include <stdbool.h>
#include <stdint.h>

typedef enum { BUTTON_OK = 0, BUTTON_ERR_PARAM } button_status_t;

/* Незмінна конфігурація. Має жити весь час роботи (static const).
 * Пін налаштовується в CubeMX: GPIO_EXTI, Rising/Falling edge, Pull-up,
 * переривання EXTI увімкнене в NVIC. */
typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin_mask;    /* GPIO_PIN_x; також номер лінії EXTI */
  bool active_low;      /* true: натиснута = 0 (кнопка на GND + pull-up) */
  uint16_t debounce_ms; /* 10..20 мс */
} button_config_t;

/* Стан екземпляра. Пам'ять виділяє викликач, поля не змінювати ззовні. */
typedef struct {
  const button_config_t *config;
  bool stable_state;                /* стан після антибрязкоту */
  bool press_event;                 /* одноразова подія натискання */
  volatile bool edge_pending;       /* пише ISR, скидає button_process */
  volatile uint32_t last_edge_tick; /* пише ISR */
} button_t;

button_status_t button_init(button_t *btn, const button_config_t *config);

/* Викликати з HAL_GPIO_EXTI_Callback для піна цієї кнопки. */
void button_irq_handler(button_t *btn);

/* Викликати з головного циклу. Завершує антибрязкіт після переривання. */
void button_process(button_t *btn);

/* Поточний стабільний стан: true, поки кнопку утримують. */
bool button_is_pressed(const button_t *btn);

/* true рівно один раз на кожне натискання. */
bool button_was_pressed(button_t *btn);