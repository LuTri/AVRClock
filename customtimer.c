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

#include "customtimer.h"
#include <avr/interrupt.h>
#include <stdlib.h>

#define MAX_TCNT 0xFFFF

#define SECONDS_PER_TICK (1024.0f / F_CPU)
#define SECONDS_PER_OVERFLOW (SECONDS_PER_TICK * MAX_TCNT)

#define MAX_SECONDS \
    (0xFFFE * SECONDS_PER_OVERFLOW) + (0xFFFE * SECONDS_PER_TICK)

CustomTimer _CT_O = {._all_steps = {0},
                     ._all_overflows = {0},
                     ._processing_steps = 0,
                     ._processing_overflows = 0,
                     ._running = 0,
                     ._act_steps = 0,
                     ._act_cycle = 0,
                     ._cycles = 0,
                     ._timer_callbacks = {0}};

uint8_t prepare_countdown(uint16_t n_cycles, float* seconds,
                          T_CALLBACK* callbacks) {
    uint16_t idx;

    if (_CT_O._running || n_cycles > MAX_COUNTDOWNS) {
        return 0;
    } else {
        _CT_O._cycles = n_cycles;

        for (idx = 0; idx < n_cycles; idx++) {
            _CT_O._timer_callbacks[idx] = callbacks[idx];

            // Number of overflows to wait for
            _CT_O._all_overflows[idx] =
                (uint16_t)(seconds[idx] / SECONDS_PER_OVERFLOW);

            // Number of non-overflow steps (compare) to wait for
            _CT_O._all_steps[idx] =
                (uint16_t)(seconds[idx] -
                           (_CT_O._all_overflows[idx] * SECONDS_PER_OVERFLOW)) /
                SECONDS_PER_TICK;
        }

        return 1;
    }
}

inline void prescale_1024(void) { TCCR1B = (1 << CS12) | (1 << CS10); }

inline void stop_timer(void) {
    TIMSK1 = 0;
    TCCR1B = 0;
}

uint8_t prepare_single_countdown(float seconds, T_CALLBACK callback) {
    float arr[1] = {seconds};
    T_CALLBACK c_arr[1] = {callback};

    return prepare_countdown(1, arr, c_arr);
}

void start_overflow_timer(void) {
    _CT_O._act_steps = 0;
    TCCR1A = 0;
    TCNT1 = 0;

    TIMSK1 = (1 << TOIE1);

    prescale_1024();
}

void start_compare_timer(void) {
    TCNT1 = 0;
    TCCR1A = 0;

    OCR1A = _CT_O._processing_steps;

    TIMSK1 = (1 << OCIE1A);

    prescale_1024();
}

void start_timer(void) {
    _CT_O._processing_steps = _CT_O._all_steps[_CT_O._act_cycle];
    _CT_O._processing_overflows = _CT_O._all_overflows[_CT_O._act_cycle];

    if (_CT_O._processing_overflows > 0) {
        start_overflow_timer();
    } else {
        start_compare_timer();
    }
}

uint8_t run_countdown(void) {
    stop_timer();

    if (_CT_O._running) {
        return 0;
    } else {
        _CT_O._running = 1;
        _CT_O._act_cycle = 0;

        start_timer();
        sei();

        return 1;
    }
}

void callback_and_next(void) {
    T_CALLBACK callback = _CT_O._timer_callbacks[_CT_O._act_cycle];
    stop_timer();

    if (!(++(_CT_O._act_cycle) == _CT_O._cycles)) {
        (*(_CT_O._timer_callbacks[0]))();
        _CT_O._running = 1;
        start_timer();
    } else {
        (*(_CT_O._timer_callbacks[0]))();
        _CT_O._running = 0;
    }

    (*callback)();
}

uint8_t check_and_inc_steps(void) {
    return (++(_CT_O._act_steps) >= _CT_O._processing_overflows);
}

ISR(TIMER1_COMPA_vect) { callback_and_next(); }

ISR(TIMER1_OVF_vect) {
    if (check_and_inc_steps()) {
        start_compare_timer();
    }
}
