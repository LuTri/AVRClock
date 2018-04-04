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

#ifndef TESTING
#include <avr/io.h>
#else
#include "test.h"
void TIMER1_COMPA_vect(void);
void TIMER1_OVF_vect(void);
#endif

#ifndef MAX_COUNTDOWNS
#define MAX_COUNTDOWNS 10
#endif

#define MAX_TCNT 0xFFFF

/*! @brief Seconds passing between 2 ticks, with a prescaler of 1024. */
#define SECONDS_PER_TICK (1024.0f / F_CPU)
/*! @brief Seconds passing between 2 overflows, with a prescaler set to 1024. */
#define SECONDS_PER_OVERFLOW (SECONDS_PER_TICK * MAX_TCNT)

/*! @brief Maximum seconds a timer can count with a prescaler set to 1024. */
#define MAX_SECONDS \
    (0xFFFE * SECONDS_PER_OVERFLOW) + (0xFFFE * SECONDS_PER_TICK)

/*!
 * Callback-function pointer typedef. All Callbacks must not have any arguments.
 */
typedef void (*T_CALLBACK)(void);

/*!
 * Structure holding all necessary information for the custom countdowns.
 * The maximum number of available countdowns is defined by \ref MAX_COUNTDOWNS,
 * which defaults to 10.
 */
typedef struct {
    /*! Number of ticks until compare interrupt for countdown #IDX is triggered.
     */
    uint16_t _cd_ticks[MAX_COUNTDOWNS];
    /*! Overflows to pass for countdown #IDX. */
    uint16_t _cd_ovfs[MAX_COUNTDOWNS];

    /*! State-flag, whether the countdown timer is currently running. */
    uint8_t _running;
    /*! Number of overflows passed so far, for currently running countdown */
    uint16_t _cur_passed_overflows;

    /*! IDX of currently running countdown. */
    uint16_t _cur_cd;
    /*! Number of configured countdowns. */
    uint16_t _n_cds;
    /*! Flags whether, a callback of countdown[x] should be run inside the ISR*/
    uint8_t _cb_in_interrupt[MAX_COUNTDOWNS];

    /*! Array containing the desired callbacks for each countdown. */
    T_CALLBACK _timer_callbacks[MAX_COUNTDOWNS];
    /*! Latest available non-ISR callback */
    T_CALLBACK ready_callback;
} CustomTimer;

/*! @brief Prepare a number of countdowns.
 *
 * **Overwrite** all existing countdowns with the parameters.
 * This function will fail, either when a countdown is currently running or
 * when the desired number of countdowns exceeds \ref MAX_COUNTDOWNS.
 * @return @c **1** on success, **0** on failure. */
uint8_t prepare_countdowns(
    uint16_t n_cds /*! Number of desired countdowns */,
    float* seconds /*! Array of length <n_cds>, containing the time for each
                  countdown */,
    T_CALLBACK*
        callbacks /*! Array of length <n_cds>, containing pointers to the
                    callbacks for each countdown */,
    uint8_t*
        cb_in_interrupt /*! Array of length <n_cds>, describing whether or
                         not the callback should be run inside the
                         interrupt routine. when NULL is given, all callbacks
                         ar run inside the interrupt routine. */);

/*! @brief Prepare a single countdown.
 *
 * This function will prepare a single countdown and **overwrite** all other
 * existing countdowns.
 * @return @c **1** on success, **0** on failure. */
uint8_t prepare_single_countdown(
    float seconds /*! desired length of the countdown */,
    T_CALLBACK callback /*! Callback for the countdown */,
    uint8_t
        cb_in_interrupt /*! flag, whether the callback should be run inside
                             the interrupt routine */);

/*! @brief Execute all configured countdowns.
 *
 * @return @c **1** when the countdown(s) have been executed, **0** when a
 * countdown is already running */
uint8_t run_countdown(void);

/*! @brief Stop and reset all configured countdowns.*/
void reset_all_countdowns(void);

/*! @brief Get latest callback
 *
 * When a callback, which is not supposed to be run inside the ISR is ready,
 * return it's pointer and set the latest callback in \ref CustomTimer to NULL.
 * @return @c \ref T_CALLBACK of the latest callback or NULL */
T_CALLBACK get_current_callback(void);

#ifdef TESTING
extern CustomTimer _CT_O;
#endif

#endif
