#ifndef _BENCHMARK_H
#define _BENCHMARK_H

#include "../AVRClock/timer.h"

#ifndef TESTING
#include <avr/io.h>
#include <avr/interrupt.h>
#else
#include "test.h"
#endif

typedef void (*PUTC_FNC_PTR)(uint8_t data);
typedef void (*PUTS_FNC_PTR)(const char* data);

void benchmark_start(void);
void benchmark_stop(uint8_t continuous);
void publish_benchmark(PUTS_FNC_PTR puts_fnc, PUTC_FNC_PTR putc_fnc, const char* msg);

#ifndef CONCAT
#define CONCAT(a, b) a##b
#endif /* ifndef CONCAT */
#ifndef CONCAT_EXP
#define CONCAT_EXP(a, b) CONCAT(a, b)
#endif /* ifndef CONCAT_EXP */

#define NAME_PUT() \
    CONCAT_EXP(put_, TIMER_DATA_TYPE)

#define PROTO_FNC_PUT() \
    void NAME_PUT() (PUTC_FNC_PTR putc_fnc, TIMER_DATA_TYPE value)


//                  value = 0xf1f8
// i: 2 ; >> = 8            0xf1
// i: 1 ; >> = 0            0x  f8

#define FNC_PUT() \
PROTO_FNC_PUT() { \
    for (uint8_t i = sizeof(TIMER_DATA_TYPE); i > 0; i--) \
        (*putc_fnc)((value >> (8 * (i - 1))) & 0xff); \
}

#define CALL_PUT(fnc, value) \
    NAME_PUT()(fnc, value);

PROTO_FNC_PUT();

#endif /* _BENCHMARK_H */