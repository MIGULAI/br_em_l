#ifndef RELAY_H
#define RELAY_H

#include <stdbool.h>

#define RELAY_PIN       GPIO_PIN_12
#define RELAY_PORT GPIOB

void relay_init(void);
bool relay_set(bool on);
bool relay_toggle(void);
bool relay_is_on(void);

#endif