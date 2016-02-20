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
	_running = false;
}

CustomTimer* CustomTimer::GetCustomTimer() {
	if (CustomTimer::_INSTANCE == 0) {
		CustomTimer::_INSTANCE = new CustomTimer();
	}

	return CustomTimer::_INSTANCE;
}

bool CustomTimer::prepare_countdown(float seconds, T_CALLBACK callback) {
	if (this->_running) {
		return false;
	} else {
		this->_timer_callback = callback;

		this->_seconds = seconds;

		this->_overflows = int(seconds / SECONDS_PER_OVERFLOW);

		this->_steps = (seconds - (this->_overflows * SECONDS_PER_OVERFLOW)) / SECONDS_PER_TICK;

		return true;
	}
}

bool CustomTimer::run_countdown(void) {
	if (this->_running) {
		return false;
	} else {
		this->_act_steps = 0;
		this->_running = true;

		if (this->_overflows > 0) {
			this->start_overflow_timer();
		} else {
			this->start_compare_timer();
		}

		return true;
	}
}

void CustomTimer::start_overflow_timer() {
	CTCCRA = 0;
	CTCNT = 0;
	CTCCRB = _BV(CCS2) | _BV(CCS0);	
	CTIFR = _BV(CTOV);
	CTIMSK = _BV(CTOIE);
}

void CustomTimer::start_compare_timer() {
	CTCCRA = 0;
	CTCNT = 0;
	COCRA = this->_steps;
	CTCCRB = _BV(CCS2) | _BV(CCS0);	
	CTIFR = _BV(COCFA);
	CTIMSK = _BV(COCIEA);
}

void CustomTimer::stop_and_callack(void) {
	CTIMSK = 0;
	CTCCRB = 0;
	this->_running = false;
	(*this->_timer_callback)();
}

bool CustomTimer::check_and_inc_steps(void) {
	return (++this->_act_steps == this->_overflows);
}


// Timer interrupt-vectors

ISR(C_TIMER_COMPA_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer();
	timer->stop_and_callack();
}

ISR(C_TIMER_OVF_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer();
	if (timer->check_and_inc_steps()) {
		timer->start_compare_timer();
	}
}
