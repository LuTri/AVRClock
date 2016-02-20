#include "customtimer.h"
#include "customtimer_config.h"
#include <avr/interrupt.h>
#include "Arduino.h"

#define _CAT(A, B) A ## B
#define CAT(A,B) _CAT(A,B)

#define CTIMSK CAT(TIMSK, TIMERUSED)
#define CTIFR CAT(TIFR, TIMERUSED)
#define CTCCRA CAT(CAT(TCCR,TIMERUSED),A)
#define CTCCRB CAT(CAT(TCCR,TIMERUSED),B)
#define COCRA CAT( CAT(OCR,TIMERUSED),A)
#define CCS2 CAT(CAT(CS,TIMERUSED),2)
#define CCS0 CAT(CAT(CS,TIMERUSED),0)

#define CTCNT CAT(TCNT, TIMERUSED)

#define CTOV CAT(TOV, TIMERUSED)
#define CTOIE CAT(TOIE, TIMERUSED)
#define COCIEA CAT(CAT(OCIE,TIMERUSED),A)

#define COCFA CAT(CAT(OCF,TIMERUSED),A)

#define C_TIMER_COMPA_vect CAT(CAT(TIMER, TIMERUSED), _COMPA_vect)
#define C_TIMER_OVF_vect CAT(CAT(TIMER, TIMERUSED), _OVF_vect)


#define SECONDS_PER_TICK (1024.0f/F_CPU)
#define SECONDS_PER_OVERFLOW (SECONDS_PER_TICK * MAX_TCNT)

CustomTimer* CustomTimer::_INSTANCE = 0;

CustomTimer::CustomTimer() {
	sei();

	_all_steps = 0;
	_all_overflows = 0;
	_timer_callbacks = 0;
	_cycles = 0;
	_act_cycle = 0;
	_running = false;
}

CustomTimer* CustomTimer::GetCustomTimer() {
	if (CustomTimer::_INSTANCE == 0) {
		CustomTimer::_INSTANCE = new CustomTimer();
	}

	return CustomTimer::_INSTANCE;
}

bool CustomTimer::prepare_countdown(
		float* seconds, int n_cycles, T_CALLBACK* callbacks) {
	if (_running) {
		return false;
	} else {

		_timer_callbacks = new T_CALLBACK[n_cycles];
		_all_steps = new unsigned int[n_cycles];
		_all_overflows = new unsigned int[n_cycles];
		_cycles = n_cycles;

		for (int idx = 0; idx < n_cycles; idx++) {

			_timer_callbacks[idx] = callbacks[idx];

			_all_overflows[idx] = int(seconds[idx] / SECONDS_PER_OVERFLOW);

			_all_steps[idx] = (seconds[idx] - \
				(_all_overflows[idx] * SECONDS_PER_OVERFLOW)) / SECONDS_PER_TICK;
		}
	
		return true;
	}
}

bool CustomTimer::run_countdown(void) {
	if (_running) {
		return false;
	} else {
		_running = true;
		_act_cycle = 0;

		start_timer();

		return true;
	}
}

void CustomTimer::start_timer(void) {
	_processing_steps = _all_steps[_act_cycle];
	_processing_overflows = _all_overflows[_act_cycle];

	if (_processing_overflows > 0) {
		start_overflow_timer();
	} else {
		start_compare_timer();
	}
}

void CustomTimer::start_overflow_timer() {
	_act_steps = 0;
	CTCCRA = 0;
	CTCNT = 0;
	CTCCRB = _BV(CCS2) | _BV(CCS0);	
	CTIFR = _BV(CTOV);
	CTIMSK = _BV(CTOIE);
}

void CustomTimer::start_compare_timer() {
	CTCCRA = 0;
	CTCNT = 0;
	COCRA = _processing_steps;
	CTCCRB = _BV(CCS2) | _BV(CCS0);	
	CTIFR = _BV(COCFA);
	CTIMSK = _BV(COCIEA);
}

void CustomTimer::callback_and_next(void) {
	CTIMSK = 0;
	CTCCRB = 0;
	(*_timer_callbacks[_act_cycle])();
	if (++_act_cycle == _cycles) {

		delete[] _timer_callbacks;
		delete[] _all_steps;
		delete[] _all_overflows;

		_timer_callbacks = 0;
		_all_steps = 0;
		_all_overflows = 0;

		_running = false;
	} else {
		start_timer();	
	}
}

bool CustomTimer::check_and_inc_steps(void) {
	return (++_act_steps == _processing_overflows);
}


// Timer interrupt-vectors

ISR(C_TIMER_COMPA_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer();
	timer->callback_and_next();
}

ISR(C_TIMER_OVF_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer();
	if (timer->check_and_inc_steps()) {
		timer->start_compare_timer();
	}
}
