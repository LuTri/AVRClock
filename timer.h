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

#ifndef _TIMER_H
#define _TIMER_H

#ifndef CONCAT
#define CONCAT(a, b) a##b
#endif
#ifndef CONCAT_EXP
#define CONCAT_EXP(a, b) CONCAT(a, b)
#endif

#define TIMER CONCAT_EXP(TIMER, TIMERNR)

/* Define register names for configured Timer */
#define T_CONTROL_A CONCAT_EXP(CONCAT_EXP(TCCR, TIMERNR), A)
#define T_CONTROL_B CONCAT_EXP(CONCAT_EXP(TCCR, TIMERNR), B)
#define T_INTERRUPT_MASK CONCAT_EXP(TIMSK, TIMERNR)
#define T_COUNTER_REGISTER CONCAT_EXP(TCNT, TIMERNR)
#define T_COMPARE_REGISTER CONCAT_EXP(CONCAT_EXP(OCR, TIMERNR), A)
#define T_OVF_I_BIT CONCAT_EXP(TOIE, TIMERNR)
#define T_CMP_I_BIT CONCAT_EXP(CONCAT_EXP(OCIE, TIMERNR), A)

/* Define thresholds for defined timer */
#if TIMERBITS == 16
#define MAX_TCNT 0xFFFF
#elif TIMERBITS == 8
#define MAX_TCNT 0xFF
#else
#error TIMERBITS can only by 8 or 16
#endif

/* Define bits for defined timer and prescaler */

/* clock selection bits */
#define CS_0 CONCAT_EXP(CONCAT_EXP(CS, TIMERNR), 0)
#define CS_1 CONCAT_EXP(CONCAT_EXP(CS, TIMERNR), 1)
#define CS_2 CONCAT_EXP(CONCAT_EXP(CS, TIMERNR), 2)

/* prescaling */
#if PRESCALER == 1024
#define PRESCALE_BITS (1 << CS_2) | (1 << CS_0)
#elif PRESCALER == 256
#define PRESCALE_BITS (1 << CS_2) | (1 << CS_1)
#elif PRESCALER == 64
#define PRESCALE_BITS (1 << CS_1) | (1 << CS_0)
#elif PRESCALER == 8
#define PRESCALE_BITS (1 << CS_1)
#elif PRESCALER == 1
#define PRESCALE_BITS (1 << CS_0)
#else
#error Unknown PRESCALER value
#endif

#ifndef TESTING
#include <avr/io.h>
#else
#include "test.h"
void CONCAT_EXP(TIMER, _COMPA_vect)(void);
void CONCAT_EXP(TIMER, _OVF_vect)(void);
#endif

/*! @brief Seconds passing between 2 ticks, with a prescaler of PRESCALER. */
#define SECONDS_PER_TICK (CONCAT_EXP(PRESCALER, .0f) / F_CPU)
/*! @brief Seconds passing between 2 overflows, with a prescaler set to
 * PRESCALER. */
#define SECONDS_PER_OVERFLOW (SECONDS_PER_TICK * MAX_TCNT)

/*! @brief Maximum seconds a timer can count with a prescaler set to PRESCALER.
 */
#define MAX_SECONDS                           \
    ((MAX_TCNT - 1) * SECONDS_PER_OVERFLOW) + \
        ((MAX_TCNT - 1) * SECONDS_PER_TICK)

#endif
