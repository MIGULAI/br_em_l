#include "bsp.h"

bool gpio_clk_enable(GPIO_TypeDef *port) {
  if (port == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    return true;
  }
#ifdef GPIOB
  if (port == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    return true;
  }
#endif
#ifdef GPIOC
  if (port == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    return true;
  }
#endif
#ifdef GPIOD
  if (port == GPIOD) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    return true;
  }
#endif
#ifdef GPIOE
  if (port == GPIOE) {
    __HAL_RCC_GPIOE_CLK_ENABLE();
    return true;
  }
#endif
#ifdef GPIOH
  if (port == GPIOH) {
    __HAL_RCC_GPIOH_CLK_ENABLE();
    return true;
  }
#endif
  return false;
}