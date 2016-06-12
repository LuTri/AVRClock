#ifndef _CUSTOMTIMER_H
#define _CUSTOMTIMER_H

#include <avr/io.h>

#ifndef MAX_COUNTDOWNS
#define MAX_COUNTDOWNS 10
#endif

typedef void (*T_CALLBACK)(void);

typedef struct {
    uint16_t _all_steps[MAX_COUNTDOWNS];
    uint16_t _all_overflows[MAX_COUNTDOWNS];

    uint16_t _processing_steps;
    uint16_t _processing_overflows;

    uint8_t _running;
    uint16_t _act_steps;

    uint16_t _act_cycle;
    uint16_t _cycles;

    T_CALLBACK _timer_callbacks[MAX_COUNTDOWNS];
} CustomTimer;

uint8_t prepare_countdown(uint16_t n_cycles,
                          float* seconds,
                          T_CALLBACK* callbacks);

uint8_t prepare_single_countdown(float seconds,
                                 T_CALLBACK callback);

uint8_t run_countdown(void);

#endif
