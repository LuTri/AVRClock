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
#ifndef TESTING
#include <avr/interrupt.h>
#else
#define inline
#include "test.h"
#endif
#include <stdlib.h>

/*! Global variable _CT_O containing countdown configuration */
CustomTimer _CT_O = {._cd_ticks = {0},
                     ._cd_ovfs = {0},
                     ._running = 0,
                     ._cur_passed_overflows = 0,
                     ._cur_cd = 0,
                     ._n_cds = 0,
                     ._timer_callbacks = {0},
                     ._cb_in_interrupt = {0},
                     .ready_callback = NULL};

/*! @brief Calculate necessary overflows and timer-ticks.
 *
 * Calculate the necessary overlows and timer-ticks for a desired time. */
void set_ovfs_and_ticks(
    float seconds /*! Desired time in seconds. */,
    uint16_t* t_ovf /*! Variable to be set to necessary overflows */,
    uint16_t* t_ticks /*! Variable to be set to necessary ticks*/) {
    uint16_t ovfs, ticks;
    ovfs = (uint16_t)(seconds / SECONDS_PER_OVERFLOW);
    ticks =
        (uint16_t)(seconds - (ovfs * SECONDS_PER_OVERFLOW)) / SECONDS_PER_TICK;

    *t_ticks = ticks;
    *t_ovf = ovfs;
}

uint8_t prepare_countdowns(uint16_t n_cds, float* seconds,
                           T_CALLBACK* callbacks, uint8_t* cb_in_interrupt) {
    uint16_t idx;

    if (_CT_O._running || n_cds > MAX_COUNTDOWNS) {
        return 0;
    } else {
        _CT_O._n_cds = n_cds;

        for (idx = 0; idx < n_cds; idx++) {
            _CT_O._timer_callbacks[idx] = callbacks[idx];

            set_ovfs_and_ticks(seconds[idx], &(_CT_O._cd_ovfs[idx]),
                               &(_CT_O._cd_ticks[idx]));
            if (cb_in_interrupt == NULL) {
                _CT_O._cb_in_interrupt[idx] = 1;
            } else {
                _CT_O._cb_in_interrupt[idx] = cb_in_interrupt[idx];
            }
        }

        return 1;
    }
}

/*! @brief Set the prescaler of TCCR1 to 1024.
 *
 * This will also cause the hardware-timer to start running. */
inline void prescale_1024(void) { TCCR1B = (1 << CS12) | (1 << CS10); }

inline void stop_timer(void) {
    TIMSK1 = 0;
    TCCR1B = 0;
}

uint8_t prepare_single_countdown(float seconds, T_CALLBACK callback,
                                 uint8_t cb_in_interrupt) {
    uint8_t cb_in_isr[1] = {cb_in_interrupt};
    float arr[1] = {seconds};
    T_CALLBACK c_arr[1] = {callback};

    return prepare_countdowns(1, arr, c_arr, cb_in_isr);
}

/*! @brief Start the timer, triggering interrupts on overflows. */
void start_overflow_timer(void) {
    _CT_O._cur_passed_overflows = 0;
    TCCR1A = 0;
    TCNT1 = 0;

    // Enable timer-overflow interrupts.
    TIMSK1 = (1 << TOIE1);

    prescale_1024();
}

/*! @brief Start the timer, triggering compare interrupts. */
void start_compare_timer(void) {
    TCNT1 = 0;
    TCCR1A = 0;

    // Set the ticks to run to before interrupting.
    OCR1A = _CT_O._cd_ticks[_CT_O._cur_cd];

    // Enable timer-compare-interrupts.
    TIMSK1 = (1 << OCIE1A);

    prescale_1024();
}

/*! @brief Execute a timer, running the countdown at _CT_O._cur_cd */
void start_timer(void) {
    if (_CT_O._cd_ovfs[_CT_O._cur_cd] > 0) {
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
        _CT_O._cur_cd = 0;

        start_timer();
        sei();

        return 1;
    }
}

void reset_all_countdowns(void) {
    uint8_t i;

    stop_timer();

    _CT_O._running = 0;
    _CT_O._cur_cd = 0;
    _CT_O._cur_passed_overflows = 0;
    _CT_O._n_cds = 0;
    _CT_O.ready_callback = NULL;

    for (i = 0; i < MAX_COUNTDOWNS; i++) {
        _CT_O._timer_callbacks[i] = NULL;
        _CT_O._cd_ovfs[i] = 0;
        _CT_O._cd_ticks[i] = 0;
    }
}

/*! @brief Execute a countdown's callback.
 *
 * **Note**: Callbacks may be run inside the ISR. When a callback is a heavy,
 * but not critical function, consider running it outside ISR (see
 * \ref prepare_countdowns ). When a next countdown is avaiable run it,
 * otherwise stop the countdown completely.
 * */
void callback_and_next(void) {
    uint8_t cur_cd = _CT_O._cur_cd;
    T_CALLBACK callback = _CT_O._timer_callbacks[cur_cd];

    stop_timer();

    if (!_CT_O._running) {
        return;
    }

    /* Execute callback in ISR or set ready_callback */
    if (!(_CT_O._cb_in_interrupt[cur_cd])) {
        _CT_O.ready_callback = callback;
    } else {
        (*callback)();
    }

    /* continue with following countdowns */
    if (!(++(_CT_O._cur_cd) == _CT_O._n_cds)) {
        _CT_O._running = 1;
        start_timer();
    } else {
        _CT_O._running = 0;
    }
}

T_CALLBACK get_current_callback(void) {
    T_CALLBACK cb;
    cb = _CT_O.ready_callback;
    _CT_O.ready_callback = NULL;

    return cb;
}

/*! @brief Increment the amount of already passed overflows.
 *
 * @return @c **0** if not all overflows have happend yet, **1** otherwise. */
uint8_t check_and_inc_steps(void) {
    return (++(_CT_O._cur_passed_overflows) >= _CT_O._cd_ovfs[_CT_O._cur_cd]);
}

#ifndef TESTING
ISR(TIMER1_COMPA_vect)
#else
void TIMER1_COMPA_vect(void)
#endif
{
    callback_and_next();
}

#ifndef TESTING
ISR(TIMER1_OVF_vect)
#else
void TIMER1_OVF_vect(void)
#endif
{
    if (check_and_inc_steps()) {
        start_compare_timer();
    }
}
