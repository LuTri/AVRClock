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

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define F_CPU 16000000

/* Mock bit definitions */
#define CS00 0b1
#define CS01 0b10
#define CS02 0b100

#define CS10 0b1
#define CS11 0b10
#define CS12 0b100

#define CS20 0b1
#define CS21 0b10
#define CS22 0b100

#define CS30 0b1
#define CS31 0b10
#define CS32 0b100

#define TOIE0 0b1
#define TOIE1 0b1
#define TOIE2 0b1
#define TOIE3 0b1

#define OCIE0A 0b10
#define OCIE1A 0b10
#define OCIE2A 0b10
#define OCIE3A 0b10

extern uint16_t T_CONTROL_A;
extern uint16_t T_CONTROL_B;
extern uint16_t T_INTERRUPT_MASK;
extern uint16_t T_COUNTER_REGISTER;
extern uint16_t T_COMPARE_REGISTER;
extern uint8_t SREG;

void sei();
void cli();
