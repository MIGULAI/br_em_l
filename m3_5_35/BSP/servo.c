#include "servo.h"
#include "bsp.h"

#define SERVO_TICK_HZ 1000000U
#define SERVO_PERIOD_US 20000U

static bool tim_clk_enable(TIM_TypeDef *tim) {
  if      (tim == TIM1)  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  else if (tim == TIM2)  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  else if (tim == TIM3)  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  else if (tim == TIM4)  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
  else if (tim == TIM5)  RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
  else if (tim == TIM9)  RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
  else if (tim == TIM10) RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
  else if (tim == TIM11) RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
  else return false;
  __DSB(); // let the clock enable settle before touching TIM registers
  return true;
}

static bool tim_on_apb2(TIM_TypeDef *tim) {
  return tim == TIM1 || tim == TIM9 || tim == TIM10 || tim == TIM11;
}

// Timer clock = PCLK if APB prescaler is 1, otherwise 2 * PCLK
static uint32_t tim_clk_hz(TIM_TypeDef *tim) {
  if (tim_on_apb2(tim)) {
    uint32_t pclk = HAL_RCC_GetPCLK2Freq();
    return (RCC->CFGR & RCC_CFGR_PPRE2_2) ? pclk * 2 : pclk;
  }
  uint32_t pclk = HAL_RCC_GetPCLK1Freq();
  return (RCC->CFGR & RCC_CFGR_PPRE1_2) ? pclk * 2 : pclk;
}

static void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, uint8_t af) {
  uint32_t afr_shift = (pin % 8) * 4;
  port->AFR[pin / 8] = (port->AFR[pin / 8] & ~(0xFU << afr_shift)) | ((uint32_t)af << afr_shift);
  port->OTYPER &= ~(1U << pin);                                          // push-pull
  port->PUPDR  &= ~(3U << (pin * 2));                                    // no pull
  port->MODER   = (port->MODER & ~(3U << (pin * 2))) | (2U << (pin * 2)); // 10 = AF mode
}

servo_status_t servo_init(servo_config_t config, struct servo_s *self) {
  if (!gpio_clk_enable(config.port) || !tim_clk_enable(config.tim)) {
    return SERVO_ERROR;
  }

  self->config = config;
  TIM_TypeDef *tim = config.tim;
  uint8_t ch = config.channel - 1; // 0..3

  // CCR1..CCR4 sit next to each other in memory
  self->ccr = &tim->CCR1 + ch;

  // 1. Time base
  tim->CR1 = 0;                                   // counter off, upcounting
  tim->PSC = tim_clk_hz(tim) / SERVO_TICK_HZ - 1; // 100 MHz / 100 = 1 MHz -> PSC = 99
  tim->ARR = SERVO_PERIOD_US - 1;                 // 20000 ticks = 20 ms
  *self->ccr = (config.min_pulse_width + config.max_pulse_width) / 2; // start at center
  self->current_angle = config.max_angle_deg / 2;

  // 2. Channel mode: CCMR1 holds CH1/CH2, CCMR2 holds CH3/CH4, 8 bits per channel
  volatile uint32_t *ccmr = (ch < 2) ? &tim->CCMR1 : &tim->CCMR2;
  uint32_t s = (ch % 2) * 8;
  *ccmr &= ~(0xFFU << s);       // CCxS = 00 -> output
  *ccmr |= (6U << (s + 4))      // OCxM = 110 -> PWM mode 1 (high while CNT < CCR)
         | (1U << (s + 3));     // OCxPE = 1 -> CCR preload (new value applies next period)

  // 3. Enable channel output, active high (4 bits per channel in CCER)
  tim->CCER &= ~(0xFU << (ch * 4));
  tim->CCER |= (1U << (ch * 4)); // CCxE = 1, CCxP = 0

  // 4. Advanced timer (TIM1): main output enable, otherwise the pin stays dead
  if (tim == TIM1) {
    tim->BDTR |= TIM_BDTR_MOE;
  }

  // 5. ARR preload + force update so PSC/ARR/CCR get loaded into the active registers
  tim->CR1 |= TIM_CR1_ARPE;
  tim->EGR = TIM_EGR_UG;

  // 6. Route the pin to the timer, then start counting
  gpio_set_af(config.port, config.pin, config.af);
  tim->CR1 |= TIM_CR1_CEN;

  return SERVO_OK;
}

void servo_set_pulse_width(struct servo_s *self, uint16_t pulse_width) {
  if (pulse_width < self->config.min_pulse_width) pulse_width = self->config.min_pulse_width;
  if (pulse_width > self->config.max_pulse_width) pulse_width = self->config.max_pulse_width;
  *self->ccr = pulse_width; // 1 tick = 1 us
}

void servo_set_angle(struct servo_s *self, uint16_t angle_deg) {
  if (angle_deg > self->config.max_angle_deg) angle_deg = self->config.max_angle_deg;
  uint32_t span = self->config.max_pulse_width - self->config.min_pulse_width;
  servo_set_pulse_width(self, self->config.min_pulse_width + span * angle_deg / self->config.max_angle_deg);
  self->current_angle = angle_deg;
}