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

#include "test.h"
#include <malloc.h>
#include <stdio.h>
#include "customtimer.h"

typedef char (*TEST_FUNC)(void);

uint16_t TCCR1A = 0;
uint16_t TCCR1B = 0;
uint16_t TIMSK1 = 0;
uint16_t TCNT1 = 0;
uint16_t OCR1A = 0;

void print_test_beauty(const char* name);
void success_message(const char* name);
void failure_message(const char* name);

char compare_dec(int yielded, int expected, const char* hint);

/* mock AVR interrupt enabling */
void sei(){};

char test_countdown_preparation(void) {
    char buf[1000];
    int result = 0;

    float seconds[] = {1.3, 7.1, MAX_SECONDS - 1, 0};
    float fail_zeros[11] = {0};

    int expected[][2] = {{0, 15625}, {1, 31250}, {65534, 46875}, {0, 0}};

    T_CALLBACK cb[] = {sei, sei};
    T_CALLBACK cb_zeros[11] = {NULL};

    print_test_beauty(__FUNCTION__);

    if ((prepare_countdowns(11, fail_zeros, cb_zeros))) {
        printf("Countdown preparation did not abort on too many countdowns!\n");
        failure_message(__FUNCTION__);
        return 1;
    }

    if (!(prepare_countdowns(4, seconds, cb))) {
        printf("Setting the countdowns failed!\n");
        failure_message(__FUNCTION__);
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        sprintf(buf, "Overflow calc for %f", seconds[i]);
        result |= compare_dec(_CT_O._cd_ovfs[i], expected[i][0], buf);
        sprintf(buf, "Tick calc for %f", seconds[i]);
        result |= compare_dec(_CT_O._cd_ticks[i], expected[i][1], buf);
        ;
    }
    if (result != 0) {
        failure_message(__FUNCTION__);
    } else {
        success_message(__FUNCTION__);
    }
    return result;
}

/* Generate the list of testfunction, add function-names here */
TEST_FUNC* collect_tests(void) {
    static TEST_FUNC funcs[] = {test_countdown_preparation, NULL};
    return funcs;
}

/* Human readable test-state messages */
void print_test_beauty(const char* name) {
    printf("Running Test \"%s\"...\n", name);
}

void success_message(const char* name) {
    printf("             \"%s\"...Success!\n", name);
}

void failure_message(const char* name) {
    printf("             \"%s\"...Failed!\n", name);
}

/* Compare helpers with humand readable output */
char compare_dec(int yielded, int expected, const char* hint) {
    if (yielded != expected) {
        printf("Failed! Expected %10d, got %10d. Hint: \"%s\"\n", expected,
               yielded, hint);
        return 1;
    }
    return 0;
}

/* returns the number of failed tests */
int main(void) {
    int idx = 0;
    float seconds = MAX_SECONDS;
    char success = 0;
    TEST_FUNC* funcs;
    TEST_FUNC func;

    funcs = collect_tests();

    while (_CT_O._cd_ovfs[0] == 0) {
        seconds -= ++idx;
        prepare_single_countdown(seconds, sei);
    }

    idx = 0;

    while ((func = funcs[idx++]) != NULL) {
        success += (*func)();
    }

    return success;
}
