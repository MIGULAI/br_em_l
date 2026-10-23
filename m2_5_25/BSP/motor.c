/**
 * motor.c — реалізація модуля керування двигуном.
 *
 * Принцип вимірювання: кожні 10 мс ШІМ вимикається на короткий час
 * (вимірювальне вікно). Після згасання струму через захисний діод
 * напруга на колекторі дорівнює Vm − ЕРС, звідки ЕРС = Vm − Vколектора.
 */
#include "motor.h"

/* ---------- Налаштування ---------- */
#define ADC_VREF_MV 3300u
#define ADC_MAX_RAW 4095u
#define DIVIDER_RATIO 3u         /* (20 кОм + 10 кОм) / 10 кОм */
#define MOTOR_RPM_PER_VOLT 1000u /* калібрувальний коефіцієнт */

#define DUTY_MAX 1000u   /* 100.0 % */
#define CYCLE_TICKS 100u /* 100 × 100 мкс = 10 мс між вимірами */

/* Мінімальне очікування у вікні. Враховує затримку оновлення CCR
 * (до одного періоду ШІМ = 200 мкс при 5 кГц) і час згасання струму. */
#define WINDOW_MIN_TICKS 4u  /* 400 мкс */
#define WINDOW_MAX_TICKS 12u /* 1.2 мс, таймаут */
#define DECAY_MARGIN_RAW 15u /* запас від шуму, відліки АЦП */
#define FILTER_DIV 4         /* згладжування: крок 1/4 */

enum { ADC_CH_COLLECTOR = 0, ADC_CH_SUPPLY, ADC_CH_COUNT };

/* ---------- Стан модуля ---------- */
static motor_config_t cfg;
static volatile uint16_t adc_buf[ADC_CH_COUNT]; /* заповнює DMA */
static volatile uint16_t target_duty;
static volatile uint32_t bemf_mv;
static uint16_t cycle_tick;
static uint16_t window_tick;
static uint8_t in_window;

/* ---------- Допоміжні функції ---------- */
static inline uint32_t raw_to_mv(uint32_t raw) {
  return raw * ADC_VREF_MV * DIVIDER_RATIO / ADC_MAX_RAW;
}

static inline void pwm_write(uint16_t permille) {
  uint32_t period = __HAL_TIM_GET_AUTORELOAD(cfg.pwm_tim) + 1u;
  __HAL_TIM_SET_COMPARE(cfg.pwm_tim, cfg.pwm_channel,
                        period * permille / DUTY_MAX);
}

static void bemf_update(uint32_t collector_raw, uint32_t supply_raw) {
  uint32_t diff_raw =
      (supply_raw > collector_raw) ? (supply_raw - collector_raw) : 0u;
  int32_t delta = (int32_t)raw_to_mv(diff_raw) - (int32_t)bemf_mv;
  bemf_mv = (uint32_t)((int32_t)bemf_mv + delta / FILTER_DIV);
}

/* Один крок вимірювального вікна. Повертає 1, коли вікно завершене. */
static uint8_t window_step(uint16_t t) {
  if (t < WINDOW_MIN_TICKS) {
    return 0u;
  }

  uint32_t col = adc_buf[ADC_CH_COLLECTOR];
  uint32_t sup = adc_buf[ADC_CH_SUPPLY];

  /* Струм через діод згас: колектор нижче живлення. */
  if (col + DECAY_MARGIN_RAW < sup) {
    bemf_update(col, sup);
    return 1u;
  }

  if (t >= WINDOW_MAX_TICKS) {
    /* Колектор ≈ живленню: двигун стоїть, ЕРС = 0.
     * Колектор вище живлення: діод ще проводить, вимір пропускаємо. */
    if (col <= sup + DECAY_MARGIN_RAW) {
      bemf_update(col, sup);
    }
    return 1u;
  }

  return 0u;
}

/* ---------- Публічний інтерфейс ---------- */
HAL_StatusTypeDef motor_init(const motor_config_t *config) {
  cfg = *config;
  target_duty = 0u;
  bemf_mv = 0u;
  cycle_tick = 0u;
  in_window = 0u;

  __HAL_TIM_SET_COMPARE(cfg.pwm_tim, cfg.pwm_channel, 0u);

  /* На STM32F4 АЦП не потребує калібрування перед запуском. */
  if (HAL_ADC_Start_DMA(cfg.adc, (uint32_t *)adc_buf, ADC_CH_COUNT) != HAL_OK)
    return HAL_ERROR;
  /* adc_buf читається напряму, колбеки DMA не потрібні. Без цього
   * переривання HT/TC приходять після кожного перетворення (~8 мкс)
   * і повністю забирають CPU у main. */
  __HAL_DMA_DISABLE_IT(cfg.adc->DMA_Handle, DMA_IT_HT | DMA_IT_TC);
  if (HAL_TIM_PWM_Start(cfg.pwm_tim, cfg.pwm_channel) != HAL_OK)
    return HAL_ERROR;
  if (HAL_TIM_Base_Start_IT(cfg.tick_tim) != HAL_OK)
    return HAL_ERROR;

  return HAL_OK;
}

void motor_set_duty(uint16_t permille) {
  target_duty = (permille > DUTY_MAX) ? DUTY_MAX : permille;
}

uint16_t motor_get_duty(void) { return target_duty; }

uint32_t motor_get_bemf_mv(void) { return bemf_mv; }

uint32_t motor_get_rpm(void) { return bemf_mv * MOTOR_RPM_PER_VOLT / 1000u; }

void motor_tick_isr(void) {
  if (++cycle_tick >= CYCLE_TICKS) {
    cycle_tick = 0u;
    window_tick = 0u;
    in_window = 1u;
  }

  if (in_window && window_step(++window_tick)) {
    in_window = 0u;
  }

  pwm_write(in_window ? 0u : target_duty);
}