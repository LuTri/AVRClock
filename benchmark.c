#include "benchmark.h"
#include "customtimer.h"

TIMER_DATA_TYPE b_start[TIMER_BENCHMARK_BUFFERS] = { 0 };
TIMER_DATA_TYPE b_end[TIMER_BENCHMARK_BUFFERS] = { 0 };

uint16_t isrs_start = 0;
uint16_t isrs_end = 0;

FNC_PUT()

void benchmark_start(void) {
    uint8_t sreg = SREG;
    cli();
    benchmark(b_start, &isrs_start, 0);
    SREG = sreg;
}

void benchmark_stop(uint8_t continuous) {
    uint8_t sreg = SREG;
    cli();
    benchmark(b_end, &isrs_end, 1);
    SREG = sreg;
}

#define CARRY_SHIFT 15
//#define CARRY_SHIFT (sizeof(TIMER_DATA_TYPE) * 8) - 1

void _apply_isrs(void) {
    uint8_t sixfour_over_TDT = sizeof(uint64_t) / sizeof(TIMER_DATA_TYPE);
    uint16_t carry, isrs;

    carry = (isrs_end < isrs_start) & (1 << CARRY_SHIFT);
    isrs = isrs_end - isrs_start;

    for (uint8_t i = 0; i < sixfour_over_TDT && (isrs || carry); i++) {
        carry -= (b_end[sixfour_over_TDT - (1 + i)] < (isrs & 0xff)) & (1 << CARRY_SHIFT);

        b_end[sixfour_over_TDT - (1 + i)] -= (isrs & 0xff);

        isrs = isrs >> 8;
        isrs += carry;
        carry = 0;
    }
}

void publish_benchmark(PUTS_FNC_PTR puts_fnc, PUTC_FNC_PTR putc_fnc, const char* msg) {
    uint8_t i;
    if (puts_fnc && msg) (*puts_fnc)(msg);

    if (putc_fnc) {
        for (i = 0; i < TIMER_BENCHMARK_BUFFERS; i++) {
            CALL_PUT(putc_fnc, b_start[i])
        }

        (*puts_fnc)("IS");
        (*putc_fnc)((isrs_start >> 8) & 0xff);
        (*putc_fnc)(isrs_start & 0xff);
        (*puts_fnc)("IE");
        (*putc_fnc)((isrs_end >> 8) & 0xff);
        (*putc_fnc)(isrs_end & 0xff);

        _apply_isrs();

        for (i = 0; i < TIMER_BENCHMARK_BUFFERS; i++) {
            CALL_PUT(putc_fnc, b_end[i])
        }
    }

    if (puts_fnc && msg) (*puts_fnc)(msg);
}
