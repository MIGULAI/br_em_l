#include <stddef.h>

#include "bsp.h"
#include "encoder.h"
#include "stm32_hal_legacy.h"
#include "stm32f4xx_hal_gpio.h"

static void pin_init(const encoder_pin_config_t *pin_config) {
  GPIO_InitTypeDef init = {0};

  gpio_clk_enable(pin_config->port);
  init.Pin = pin_config->pin;
  init.Pull = GPIO_PULLUP;
#if defined(STM32F1)
  /* F1: немає регістрів AFR і поля Alternate. Входи периферії
   * постійно підключені до своїх пінів, тому достатньо режиму Input.
   * Інші піни лише через ремап AFIO (__HAL_AFIO_REMAP_xxx). */
  init.Mode = GPIO_MODE_INPUT;
#else
  /* F2/F4 і новіші: пін підключається до таймера через AF-мультиплексор. */
  init.Mode = GPIO_MODE_AF_PP;
  init.Speed = GPIO_SPEED_FREQ_LOW;
  init.Alternate = pin_config->alternate;
#endif
  HAL_GPIO_Init(pin_config->port, &init);
}

static bool config_is_valid(encoder_config_t *cfg) {
  return (cfg != NULL) &&
         (cfg->ch_a_config.port != NULL) &&
         (cfg->ch_b_config.port != NULL) &&
         (cfg->counts_per_detent > 0U) &&
         (cfg->input_filter <= ENCODER_FILTER_MAX);
}

encoder_status_t encoder_init(encoder_t *enc, encoder_config_t *config) {
  /* Налаштування вхідних каналів таймера для режиму енкодера.
   * {0} обнуляє всі поля, щоб не лишилося випадкових значень зі стеку. */
  TIM_Encoder_InitTypeDef enc_hw_cfg = {0};

  /* ---------- Перевірка вхідних даних ---------- */
  if ((enc == NULL) || !config_is_valid(config)) {
    return ENCODER_ERR_PARAM;
  }
  /* Без тактування таймер не реагує на запис у регістри.
   * false = таймер не підтримується драйвером. */
  if (!tim_clock_enable(config->tim_instance)) {
    return ENCODER_ERR_PARAM;
  }

  /* ---------- Стан драйвера ---------- */
  enc->config = *config;
  enc->last_count = 0; /* останнє прочитане значення лічильника */
  enc->residual = 0;   /* відліки, що ще не склали повний клік */

  /* ---------- Піни ---------- */
  /* CLK -> TIMx_CH1, DT -> TIMx_CH2, режим AF + pull-up */
  pin_init(&config->ch_a_config);
  pin_init(&config->ch_b_config);

  /* ---------- База таймера (лічильник) ---------- */
  enc->tim_handle.Instance = config->tim_instance;

  /* Дільник лічильника: 0 = рахувати кожен фронт від енкодера.
   * Значення N рахувало б лише кожен (N+1)-й фронт, тобто кліки губилися б. */
  enc->tim_handle.Init.Prescaler = 0U;

  /* У режимі енкодера напрямок рахування (вгору/вниз) визначає апаратно
   * фазовий зсув між A і B. Поле обов'язкове для HAL, але на роботу
   * енкодера не впливає. */
  enc->tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;

  /* ARR, межа лічильника: рахує 0..65535 і далі по колу.
   * Повний 16-бітний діапазон потрібен, щоб різниця двох значень
   * у int16_t коректно обробляла переповнення (65535 -> 0). */
  enc->tim_handle.Init.Period = ENCODER_COUNTER_PERIOD;

  /* Частота вибірки цифрового фільтра (fDTS) відносно частоти таймера.
   * DIV1: fDTS = fTIM (100 МГц на F411). DIV2/DIV4 уповільнюють вибірку,
   * тобто фільтр стає "довшим". */
  enc->tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

  /* Буферизація ARR: при DISABLE новий Period застосовується одразу,
   * при ENABLE лише після переповнення. Period не змінюється під час
   * роботи, тому значення не має практичного впливу. */
  enc->tim_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  /* ---------- Режим енкодера ---------- */
  /* Які фронти рахувати:
   *   TI1  - лише фронти каналу A  -> 2 відліки на клік KY-040
   *   TI2  - лише фронти каналу B  -> 2 відліки на клік
   *   TI12 - фронти обох каналів   -> 4 відліки на клік (x4, найточніше
   *          і найстійкіше до брязкоту, бо стан перевіряється по обох) */
  enc_hw_cfg.EncoderMode = TIM_ENCODERMODE_TI12;

  /* ---- Канал 1 (A / CLK) ---- */
  /* У режимі енкодера це не "рахувати лише фронт, що наростає",
   * а полярність сигналу: RISING = без інверсії, FALLING = інверсія.
   * Інверсія одного каналу змінює напрямок рахування на протилежний
   * (альтернатива перестановці проводів A/B). */
  enc_hw_cfg.IC1Polarity = TIM_ICPOLARITY_RISING;

  /* Звідки канал бере сигнал: DIRECTTI = зі свого піна (CH1 -> IC1).
   * Для режиму енкодера лише так. INDIRECTTI підмінив би вхід
   * сигналом сусіднього каналу. */
  enc_hw_cfg.IC1Selection = TIM_ICSELECTION_DIRECTTI;

  /* Дільник подій захоплення. Для енкодера має бути DIV1,
   * інакше частина фронтів ігнорується. */
  enc_hw_cfg.IC1Prescaler = TIM_ICPSC_DIV1;

  /* Цифровий фільтр, 0..15. Фронт приймається, лише якщо новий рівень
   * стабільний N вибірок поспіль на частоті fSAMPLING.
   *   0  - без фільтра
   *   15 - fSAMPLING = fDTS/32, N = 8
   * На F411 (fDTS = 100 МГц): 100 МГц / 32 = 3.125 МГц,
   * 8 вибірок = ~2.56 мкс. Прибирає короткі завади, але механічний
   * брязкіт KY-040 триває до одиниць мс, тому проти нього
   * ефективніші RC-ланцюжки (10 кОм + 10..100 нФ). */
  enc_hw_cfg.IC1Filter = config->input_filter;

  /* ---- Канал 2 (B / DT): значення ті самі, що й для каналу 1 ---- */
  enc_hw_cfg.IC2Polarity = TIM_ICPOLARITY_RISING;
  enc_hw_cfg.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  enc_hw_cfg.IC2Prescaler = TIM_ICPSC_DIV1;
  enc_hw_cfg.IC2Filter = config->input_filter;

  /* ---------- Запуск ---------- */
  /* Записує налаштування в регістри (CR1, ARR, PSC, SMCR, CCMR1, CCER).
   * Викликає слабку HAL_TIM_Encoder_MspInit(): якщо CubeMX згенерував її
   * для цього таймера, вона перезапише налаштування пінів. */
  if (HAL_TIM_Encoder_Init(&enc->tim_handle, &enc_hw_cfg) != HAL_OK) {
    return ENCODER_ERR_HAL;
  }

  /* Вмикає обидва канали (CC1E, CC2E) і сам лічильник (CEN).
   * Після цього таймер рахує повністю апаратно, без участі CPU. */
  if (HAL_TIM_Encoder_Start(&enc->tim_handle, TIM_CHANNEL_ALL) != HAL_OK) {
    return ENCODER_ERR_HAL;
  }

  /* Синхронізація з поточним значенням лічильника,
   * щоб перший виклик GetSteps не повернув хибний крок. */
  enc->last_count = (int16_t)__HAL_TIM_GET_COUNTER(&enc->tim_handle);

  return ENCODER_OK;
}

int32_t encoder_get_step(encoder_t *enc) {
  const int16_t now = (int16_t)__HAL_TIM_GET_COUNTER(&enc->tim_handle);
  const int16_t diff = (int16_t)(now - enc->last_count);
  const int16_t cpd = enc->config.counts_per_detent;

  int32_t steps;

  enc->last_count = now;
  enc->residual += diff;
  steps = enc->residual / cpd;
  enc->residual -= steps * cpd;

  return enc->config.invert_direction ? -steps : steps;
}

void encoder_reset(encoder_t *encoder)
{
    __HAL_TIM_SetCounter(&encoder->tim_handle, 0U);
    encoder->last_count = 0;
    encoder->residual = 0;
}