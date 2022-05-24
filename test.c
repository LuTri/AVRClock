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

uint8_t SREG = 0;
uint16_t T_CONTROL_A = 0;
uint16_t T_CONTROL_B = 0;
uint16_t T_INTERRUPT_MASK = 0;
uint16_t T_COUNTER_REGISTER = 0;
uint16_t T_COMPARE_REGISTER = 0;

void print_test_beauty(const char* name);
void success_message(const char* name);
void failure_message(const char* name);

char compare_dec(int yielded, int expected, const char* hint);
char compare_cb(T_CALLBACK yielded, T_CALLBACK expected, const char* hint);

/* mock AVR interrupt enabling */
void sei(){};
/* mock AVR interrupt disabling */
void cli(){};

/*
 * Test functions
 */

/* time exceeding testing */

char test_failure_on_exceeding(void) {
    char result = 0;
    float seconds = MAX_SECONDS + 1;

    print_test_beauty(__FUNCTION__);

    if ((prepare_single_countdown(seconds, sei, 0))) {
        printf(
            "Countdown preparation did not abort while exceeding "
            "MAX_SECONDS!\n");
        failure_message(__FUNCTION__);
        return 1;
    }

    if (result != 0) {
        failure_message(__FUNCTION__);
    } else {
        success_message(__FUNCTION__);
    }
    return result;
}

/* preparation testing */

char test_countdown_preparation(void) {
    int i;
    char buf[1000];
    char result = 0;

    float seconds[4];
    int expected[4][2];

    float fail_zeros[11] = {0};

    for (i = 0; i < 4; i++) {
        seconds[i] = (MAX_SECONDS - 1) / 3 * i;
        expected[i][0] = (int)(seconds[i] / SECONDS_PER_OVERFLOW);
        expected[i][1] =
            (int)(seconds[i] - (expected[i][0] * SECONDS_PER_OVERFLOW)) /
            SECONDS_PER_TICK;
    }

    T_CALLBACK cb[] = {sei, sei};
    T_CALLBACK cb_zeros[11] = {NULL};

    print_test_beauty(__FUNCTION__);

    if ((prepare_countdowns(11, fail_zeros, cb_zeros, NULL))) {
        printf("Countdown preparation did not abort on too many countdowns!\n");
        failure_message(__FUNCTION__);
        return 1;
    }

    if (!(prepare_countdowns(4, seconds, cb, NULL))) {
        printf("Setting the countdowns failed!\n");
        failure_message(__FUNCTION__);
        return 1;
    }

    for (i = 0; i < 4; i++) {
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

/* Mocked callbacks */
int value = 0;
void set_value_to_1(void) { value = 1; }
void set_value_to_2(void) { value = 2; }
void set_value_to_3(void) { value = 3; }

/* callback testing */

char test_countdown_callback(void) {
    char result = 0;

    float secs[] = {0, 0};
    T_CALLBACK callbacks[] = {set_value_to_1, set_value_to_2};

    print_test_beauty(__FUNCTION__);

    if (!(prepare_countdowns(2, secs, callbacks, NULL))) {
        printf("Setting the countdowns failed!\n");
        failure_message(__FUNCTION__);
        return 1;
    }
    run_countdown();
    result |= compare_dec(value, 0, "PRE Callback execution");
    CONCAT_EXP(TIMER, _COMPA_vect)();
    result |= compare_dec(value, 1, "Callback execution 1");
    CONCAT_EXP(TIMER, _COMPA_vect)();
    result |= compare_dec(value, 2, "Callback execution 2");

    if (result != 0) {
        failure_message(__FUNCTION__);
    } else {
        success_message(__FUNCTION__);
    }
    return result;
}

/* reset testing */

char test_reset_countdowns(void) {
    char result = 0;

    float secs[] = {0, 0};
    T_CALLBACK callbacks[] = {set_value_to_1, set_value_to_2};

    reset_all_countdowns();
    print_test_beauty(__FUNCTION__);

    value = 0;
    if (!(prepare_countdowns(2, secs, callbacks, NULL))) {
        printf("Setting the countdowns failed!\n");
        failure_message(__FUNCTION__);
        return 1;
    }
    run_countdown();
    result |= compare_dec(value, 0, "PRE Callback execution");
    CONCAT_EXP(TIMER, _COMPA_vect)();
    result |= compare_dec(value, 1, "Callback execution 1");
    reset_all_countdowns();
    CONCAT_EXP(TIMER, _COMPA_vect)();
    result |= compare_dec(value, 1, "Callback execution 2");

    if (result != 0) {
        failure_message(__FUNCTION__);
    } else {
        success_message(__FUNCTION__);
    }
    return result;
}

/* test non-ISR callbacks */

char test_non_isr_callback_combination(void) {
    char result = 0;
    T_CALLBACK cur_cb;

    float secs[] = {0, 0, 0};
    T_CALLBACK callbacks[] = {set_value_to_1, set_value_to_2, set_value_to_3};
    uint8_t in_isr[] = {1, 0, 1};

    reset_all_countdowns();
    print_test_beauty(__FUNCTION__);

    value = 0;
    if (!(prepare_countdowns(3, secs, callbacks, in_isr))) {
        printf("Setting the countdowns failed!\n");
        failure_message(__FUNCTION__);
        return 1;
    }

    run_countdown();

    result |= compare_dec(value, 0, "PRE Callback execution");
    CONCAT_EXP(TIMER, _COMPA_vect)();
    compare_cb(get_current_callback(), NULL, "Available callback 1");
    result |= compare_dec(value, 1, "Callback execution 1");

    CONCAT_EXP(TIMER, _COMPA_vect)();
    result |= compare_dec(value, 1, "Callback execution 2");

    cur_cb = get_current_callback();
    result |= compare_cb(cur_cb, set_value_to_2, "Available callback 2");
    (*cur_cb)();
    result |= compare_dec(value, 2, "NON-ISR Callback execution 2");

    CONCAT_EXP(TIMER, _COMPA_vect)();
    compare_cb(get_current_callback(), NULL, "Available callback 3");
    result |= compare_dec(value, 3, "Callback execution 3");

    if (result != 0) {
        failure_message(__FUNCTION__);
    } else {
        success_message(__FUNCTION__);
    }
    return result;
}

/*
 * !Test functions
 */

/* Generate the list of testfunction, add function-names here */
TEST_FUNC* collect_tests(void) {
    static TEST_FUNC funcs[] = {
        test_failure_on_exceeding,         test_countdown_preparation,
        test_countdown_callback,           test_reset_countdowns,
        test_non_isr_callback_combination, NULL /* Array end */
    };
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

char compare_cb(T_CALLBACK yielded, T_CALLBACK expected, const char* hint) {
    if (yielded != expected) {
        printf("Failed! Expected %p, got %p. Hint: \"%s\"\n", expected, yielded,
               hint);
        return 1;
    }
    return 0;
}

/* returns the number of failed tests */
int main(void) {
    int idx = 0;
    int success = 0;

    TEST_FUNC* funcs;
    TEST_FUNC func;

    funcs = collect_tests();

    while ((func = funcs[idx++]) != NULL) {
        success += (*func)();
    }

    if (success == 0) {
        printf("Awesome, all tests succeeded!\n");
    } else {
        printf("%d tests failed.\n", success);
    }

    return success;
}
