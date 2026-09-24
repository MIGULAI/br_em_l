#include "bsp.h"

bool gpio_clk_enable(GPIO_TypeDef *port)
{
    if (port == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); return true; }
#ifdef GPIOB
    if (port == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOC
    if (port == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOD
    if (port == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOE
    if (port == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOH
    if (port == GPIOH) { __HAL_RCC_GPIOH_CLK_ENABLE(); return true; }
#endif
    return false;
}

bool tim_clock_enable(TIM_TypeDef *tim)
{
#ifdef TIM1
    if (tim == TIM1) { __HAL_RCC_TIM1_CLK_ENABLE(); return true; }
#endif
#ifdef TIM2
    if (tim == TIM2) { __HAL_RCC_TIM2_CLK_ENABLE(); return true; }
#endif
#ifdef TIM3
    if (tim == TIM3) { __HAL_RCC_TIM3_CLK_ENABLE(); return true; }
#endif
#ifdef TIM4
    if (tim == TIM4) { __HAL_RCC_TIM4_CLK_ENABLE(); return true; }
#endif
#ifdef TIM5
    if (tim == TIM5) { __HAL_RCC_TIM5_CLK_ENABLE(); return true; }
#endif
    return false;
}
 