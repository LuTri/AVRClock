/*
 * This file is part of the AVRClock distribution
 * (https://github.com/LuTri/AVRClock).
 * Copyright (c) 2016 Tristan Lucas.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
