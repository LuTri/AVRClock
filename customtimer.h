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

    /*! Array containing the desired callbacks for each countdown. */
    T_CALLBACK _timer_callbacks[MAX_COUNTDOWNS];
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
                    callbacks for each countdown */);

/*! @brief Prepare a single countdown.
 *
 * This function will prepare a single countdown and **overwrite** all other
 * existing countdowns.
 * @return @c **1** on success, **0** on failure. */
uint8_t prepare_single_countdown(
    float seconds /*! desired length of the countdown */,
    T_CALLBACK callback /*! Callback for the countdown */);

/*! @brief Execute all configured countdowns.
 *
 * @return @c **1** when the countdown(s) have been executed, **0** when a
 * countdown is already running */
uint8_t run_countdown(void);

/*! @brief Stop and reset all configured countdowns.*/
void reset_all_countdowns(void);

#endif
