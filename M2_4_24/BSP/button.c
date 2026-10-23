#include "button.h"
#include <stddef.h>
#include <stdio.h>

static bool read_pressed(const button_config_t *cfg) {
  const GPIO_PinState level = HAL_GPIO_ReadPin(cfg->port, cfg->pin_mask);
  return cfg->active_low ? (level == GPIO_PIN_RESET) : (level == GPIO_PIN_SET);
}

/* Регістр EXTI->IMR спільний для всіх ліній. Read-modify-write з головного
 * циклу може перервати ISR іншої кнопки, і її зміна загубиться.
 * Тому модифікація в критичній секції зі збереженням стану PRIMASK. */
static void exti_line_enable(uint16_t pin_mask) {
  const uint32_t primask = __get_PRIMASK();
  __disable_irq();
  EXTI->IMR |= pin_mask;
  __set_PRIMASK(primask);
}

button_status_t button_init(button_t *btn, const button_config_t *config) {
  if ((btn == NULL) || (config == NULL) || (config->port == NULL)) {
    return BUTTON_ERR_PARAM;
  }

  btn->config = config;
  btn->stable_state = read_pressed(config); /* без хибної події при старті */
  btn->press_event = false;
  btn->edge_pending = false;
  btn->last_edge_tick = HAL_GetTick();
  return BUTTON_OK;
}

/* Контекст переривання: мінімум роботи, без printf і очікувань. */
void button_irq_handler(button_t *btn) {
  if ((btn == NULL) || (btn->config == NULL)) {
    return; /* ще не ініціалізовано: HAL уже скинув прапорець, ігноруємо */
  }
  /* Блокування лінії на час антибрязкоту: один фронт = одне переривання
   * замість десятків від брязкоту контактів. У ISR лінія вже не може
   * бути перервана тією ж лінією, тому RMW тут безпечний. */
  EXTI->IMR &= ~(uint32_t)btn->config->pin_mask;

  btn->last_edge_tick = HAL_GetTick();
  btn->edge_pending = true;
}

void button_process(button_t *btn) {
  bool now;

  if (!btn->edge_pending) {
    return;
  }
  if ((HAL_GetTick() - btn->last_edge_tick) < btn->config->debounce_ms) {
    return; /* вікно антибрязкоту ще триває */
  }

  /* Порядок важливий: спочатку повторно ввімкнути переривання, потім
   * читати рівень. Фронт після ввімкнення запустить новий цикл,
   * тому зміна рівня між читанням і ввімкненням не загубиться. */
  btn->edge_pending = false;
  __HAL_GPIO_EXTI_CLEAR_IT(
      btn->config->pin_mask); /* фронти брязкоту, що накопичились */
  exti_line_enable(btn->config->pin_mask);

  now = read_pressed(btn->config);
  if (now != btn->stable_state) {
    btn->stable_state = now;
    if (now) {
      btn->press_event = true;
    }
  }
}

bool button_is_pressed(const button_t *btn) { return btn->stable_state; }

bool button_was_pressed(button_t *btn) {
  const bool event = btn->press_event;
  btn->press_event = false;
  return event;
}
