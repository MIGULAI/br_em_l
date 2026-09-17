#pragma once
#include <stdbool.h>
#include <stdint.h>


void     bsp_init();
void     bsp_led_write(bool on);
void     bsp_led_toggle(void);
uint32_t bsp_millis(void);