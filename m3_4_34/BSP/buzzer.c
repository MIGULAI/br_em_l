#include "buzzer.h"

void buzzer_tone(uint32_t freq_hz, uint8_t duty_pct)
{
    if (freq_hz == 0 || duty_pct == 0) {           // silence
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        return;
    }
    uint32_t arr = BUZZER_TICK_HZ / freq_hz - 1;
    uint32_t ccr = (arr + 1) * duty_pct / 100;
    __HAL_TIM_SET_AUTORELOAD(&htim2, arr);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, ccr);
}


static const note_t *seq; 
static uint8_t len, idx; 
static uint32_t t0;

void buzzer_play(const note_t *s, uint8_t n)
{
    seq = s; len = n; idx = 0; t0 = HAL_GetTick();
    buzzer_tone(seq[0].freq, 50);
}

void buzzer_update(void)            // call from app_run()
{
    if (!seq) return;
    if (HAL_GetTick() - t0 < seq[idx].ms) return;
    if (++idx >= len) { buzzer_tone(0, 0); seq = NULL; return; }
    t0 = HAL_GetTick();
    buzzer_tone(seq[idx].freq, 50);
}