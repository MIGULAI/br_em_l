#include "tim.h"

#define BUZZER_TICK_HZ 1000000u   // 100 MHz / (PSC + 1)

#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_DS4 311
#define NOTE_AS4 466
#define NOTE_D5  587
#define NOTE_DS5 622

#define GAP {0, 20}   // short silence so repeated notes don't merge into one tone


typedef struct { uint16_t freq; uint16_t ms; } note_t;   // freq 0 = pause

void buzzer_tone(uint32_t freq_hz, uint8_t duty_pct);
void buzzer_play(const note_t *s, uint8_t n);
void buzzer_update(void);