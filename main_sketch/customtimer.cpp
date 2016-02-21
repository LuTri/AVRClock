#include "customtimer.h"
#include "customtimer_config.h"
#include <avr/interrupt.h>

#define MAX_TCNT 0xFFFF

#define _CAT(A, B) A ## B
#define CAT(A,B) _CAT(A,B)

//REGISTERS FIRST TIMER

#define C1TIMSK CAT(TIMSK, TIMER_1ST)
#define C1TIFR CAT(TIFR, TIMER_1ST)
#define C1TCCRA CAT(CAT(TCCR,TIMER_1ST),A)
#define C1TCCRB CAT(CAT(TCCR,TIMER_1ST),B)
#define C1OCRA CAT( CAT(OCR,TIMER_1ST),A)
#define C1TCNT CAT(TCNT, TIMER_1ST)

//REGISTERS SECOND TIMER

#define C2TIMSK CAT(TIMSK, TIMER_2ND)
#define C2TIFR CAT(TIFR, TIMER_2ND)
#define C2TCCRA CAT(CAT(TCCR,TIMER_2ND),A)
#define C2TCCRB CAT(CAT(TCCR,TIMER_2ND),B)
#define C2OCRA CAT( CAT(OCR,TIMER_2ND),A)
#define C2TCNT CAT(TCNT, TIMER_2ND)

// VALUES FIRST TIMER

#define C1CS2 CAT(CAT(CS,TIMER_1ST),2)
#define C1CS0 CAT(CAT(CS,TIMER_1ST),0)
#define C1TOV CAT(TOV, TIMER_1ST)
#define C1TOIE CAT(TOIE, TIMER_1ST)
#define C1OCIEA CAT(CAT(OCIE,TIMER_1ST),A)
#define C1OCFA CAT(CAT(OCF,TIMER_1ST),A)

// VALUES SECOND TIMER

#define C2CS2 CAT(CAT(CS,TIMER_2ND),2)
#define C2CS0 CAT(CAT(CS,TIMER_2ND),0)
#define C2TOV CAT(TOV, TIMER_2ND)
#define C2TOIE CAT(TOIE, TIMER_2ND)
#define C2OCIEA CAT(CAT(OCIE,TIMER_2ND),A)
#define C2OCFA CAT(CAT(OCF,TIMER_2ND),A)

// INTERRUPT VECTORS

#define C_TIMER_1ST_COMPA_vect CAT(CAT(TIMER, TIMER_1ST), _COMPA_vect)
#define C_TIMER_1ST_OVF_vect CAT(CAT(TIMER, TIMER_1ST), _OVF_vect)

#define C_TIMER_2ND_COMPA_vect CAT(CAT(TIMER, TIMER_2ND), _COMPA_vect)
#define C_TIMER_2ND_OVF_vect CAT(CAT(TIMER, TIMER_2ND), _OVF_vect)



#define SECONDS_PER_TICK (1024.0f/F_CPU)
#define SECONDS_PER_OVERFLOW (SECONDS_PER_TICK * MAX_TCNT)


//volatile uint16_t* FOO = &CTCNT;

enum REGISTERS8BIT {CTIMSK=0, CTIFR=1, CTCCRA=2, CTCCRB=3};
enum REGISTERS16BIT {COCRA=0, CTCNT=1};
enum REG_VALUES {CCS2=0, CCS0=1, CTOV=2, CTOIE=3, COCIEA=4, COCFA=5};

CustomTimer* CustomTimer::_INSTANCE_2ND = 0;
CustomTimer* CustomTimer::_INSTANCE_1ST = 0;

CustomTimer::CustomTimer(bool is_first) {
	sei();

	_all_steps = 0;
	_all_overflows = 0;
	_timer_callbacks = 0;
	_cycles = 0;
	_act_cycle = 0;
	_running = false;
	fill_reg_values(is_first);
}

CustomTimer* CustomTimer::GetCustomTimer(int n_timer) {
	if (n_timer == 0) {
		if (CustomTimer::_INSTANCE_1ST == 0) {
			CustomTimer::_INSTANCE_1ST = new CustomTimer(true);
		}

		return CustomTimer::_INSTANCE_1ST;
	} else {
		if (CustomTimer::_INSTANCE_2ND == 0) {
			CustomTimer::_INSTANCE_2ND = new CustomTimer(false);
		}

		return CustomTimer::_INSTANCE_2ND;
	}
}

void CustomTimer::fill_reg_values(bool is_first) {
	if (is_first) {
		_registers8bit[CTIMSK] = &C1TIMSK;
		_registers8bit[CTIFR] = &C1TIFR;
		_registers8bit[CTCCRA] = &C1TCCRA;
		_registers8bit[CTCCRB] = &C1TCCRB;

		_registers16bit[COCRA] = &C1OCRA;
		_registers16bit[CTCNT] = &C1TCNT;

		_register_values[CCS2] = C1CS2;	
		_register_values[CCS0] = C1CS0;	
		_register_values[CTOV] = C1TOV;	
		_register_values[CTOIE] = C1TOIE;	
		_register_values[COCIEA] = C1OCIEA;	
		_register_values[COCFA] = C1OCFA;	
	} else {
		_registers8bit[CTIMSK] = &C2TIMSK;
		_registers8bit[CTIFR] = &C2TIFR;
		_registers8bit[CTCCRA] = &C2TCCRA;
		_registers8bit[CTCCRB] = &C2TCCRB;

		_registers16bit[COCRA] = &C2OCRA;
		_registers16bit[CTCNT] = &C2TCNT;

		_register_values[CCS2] = C2CS2;	
		_register_values[CCS0] = C2CS0;	
		_register_values[CTOV] = C2TOV;	
		_register_values[CTOIE] = C2TOIE;	
		_register_values[COCIEA] = C2OCIEA;	
		_register_values[COCFA] = C2OCFA;	
	}
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

bool CustomTimer::prepare_countdown(float seconds, T_CALLBACK callback) {
	float t_sec[1] = { seconds };
	T_CALLBACK t_callback[1] = { callback };

	prepare_countdown(t_sec, 1, t_callback);
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
	*_registers8bit[CTCCRA] = 0;
	*_registers16bit[CTCNT] = 0;
	*_registers8bit[CTCCRB] = _BV(_register_values[CCS2]) | _BV(_register_values[CCS0]);	
	*_registers8bit[CTIFR] = _BV(_register_values[CTOV]);
	*_registers8bit[CTIMSK] = _BV(_register_values[CTOIE]);
}

void CustomTimer::start_compare_timer() {
	*_registers8bit[CTCCRA] = 0;
	*_registers16bit[CTCNT] = 0;
	*_registers16bit[COCRA] = _processing_steps;
	*_registers8bit[CTCCRB] = _BV(_register_values[CCS2]) | _BV(_register_values[CCS0]);	
	*_registers8bit[CTIFR] = _BV(_register_values[COCFA]);
	*_registers8bit[CTIMSK] = _BV(_register_values[COCIEA]);
}

void CustomTimer::callback_and_next(void) {
	*_registers8bit[CTIMSK] = 0;
	*_registers8bit[CTCCRB] = 0;

	T_CALLBACK callback = _timer_callbacks[_act_cycle];

	if (!(++_act_cycle == _cycles)) {
		_running = true;
		start_timer();	
	} else {
		_running = false;
		delete[] _timer_callbacks;
		delete[] _all_steps;
		delete[] _all_overflows;

		_timer_callbacks = 0;
		_all_steps = 0;
		_all_overflows = 0;
	}

	(*callback)();
}

bool CustomTimer::check_and_inc_steps(void) {
	return (++_act_steps == _processing_overflows);
}


// Timer interrupt-vectors
// First timer

ISR(C_TIMER_1ST_COMPA_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer(0);
	timer->callback_and_next();
}

ISR(C_TIMER_1ST_OVF_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer(0);
	if (timer->check_and_inc_steps()) {
		timer->start_compare_timer();
	}
}

// Second timer

ISR(C_TIMER_2ND_COMPA_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer(1);
	timer->callback_and_next();
}

ISR(C_TIMER_2ND_OVF_vect) {
	CustomTimer* timer = CustomTimer::GetCustomTimer(1);
	if (timer->check_and_inc_steps()) {
		timer->start_compare_timer();
	}
}

