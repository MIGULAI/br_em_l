/**
 * motor.h — керування колекторним двигуном через ключ на нижньому плечі
 *           (2N2222A / MOSFET) з вимірюванням швидкості за зворотною ЕРС.
 *
 * Апаратне налаштування (STM32F4, CubeMX).
 * f_tim — тактова частота таймерів APB1 (дивіться Clock Configuration):
 *   STM32F401 / F407 / F446 при типових налаштуваннях — 84 МГц,
 *   STM32F411 при 100 МГц — 100 МГц.
 *
 *   TIM2 CH1 (PA0)  — PWM Generation, Prescaler = 0,
 *                     Period = f_tim / 5000 − 1 (5 кГц):
 *                     84 МГц → 16799, 100 МГц → 19999
 *   TIM4            — Internal Clock, Prescaler = f_tim / 1 МГц − 1
 *                     (84 МГц → 83, 100 МГц → 99), Period = 99 (10 кГц),
 *                     NVIC: TIM4 global interrupt = Enabled
 *   ADC1            — IN4 (PA4, дільник з колектора),
 *                     IN5 (PA5, дільник з шини живлення двигуна)
 *                     Scan Conversion Mode = Enabled,
 *                     Continuous Conversion Mode = Enabled,
 *                     DMA Continuous Requests = Enabled (обов'язково!),
 *                     Rank 1 = IN4, Rank 2 = IN5, Sampling = 56 cycles
 *   DMA (ADC1)      — DMA2 Stream0, Circular, Half Word
 *   Дільники        — 20 кОм (верхній) / 10 кОм (нижній), коефіцієнт 3
 */
#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct {
    TIM_HandleTypeDef *pwm_tim;      /* таймер ШІМ */
    uint32_t           pwm_channel;  /* канал ШІМ */
    TIM_HandleTypeDef *tick_tim;     /* таймер тіків 10 кГц */
    ADC_HandleTypeDef *adc;          /* АЦП у режимі scan + DMA */
} motor_config_t;

/* Ініціалізація модуля. Двигун після неї зупинений. */
HAL_StatusTypeDef motor_init(const motor_config_t *config);

/* Шпаруватість у проміле: 0 = стоп, 1000 = повна швидкість. */
void     motor_set_duty(uint16_t permille);
uint16_t motor_get_duty(void);

/* Згладжена зворотна ЕРС, мВ. */
uint32_t motor_get_bemf_mv(void);

/* Оберти за хвилину (потребує калібрування MOTOR_RPM_PER_VOLT). */
uint32_t motor_get_rpm(void);

/* Викликати з переривання таймера тіків (TIM4). */
void motor_tick_isr(void);

#endif /* MOTOR_H */