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

typedef unsigned long uint16_t;
typedef unsigned char uint8_t;

#define F_CPU 16000000

#define CS10 0b1
#define CS12 0b11
#define TOIE0 0b1
#define TOIE1 0b10
#define OCIE1A 0b1

#define ISR(vector) void vector(void)

extern uint16_t TCCR1A;
extern uint16_t TCCR1B;
extern uint16_t TIMSK1;
extern uint16_t TCNT1;
extern uint16_t OCR1A;

void sei();
