#pragma once
#include <stdbool.h>
#include "gpio.h"

bool gpio_clk_enable(GPIO_TypeDef *port);
bool tim_clock_enable(TIM_TypeDef *tim);