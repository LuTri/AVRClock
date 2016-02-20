#ifndef _CUSTOMTIMER_H
#define _CUSTOMTIMER_H

typedef void (*T_CALLBACK)(void);

class CustomTimer {
private:
	bool _running;

	unsigned int _act_steps;

	unsigned int _steps;
	unsigned int _overflows;
	float _seconds;

	static CustomTimer* _INSTANCE;
	T_CALLBACK _timer_callback;

	CustomTimer();

public:
	static CustomTimer* GetCustomTimer();

	bool prepare_countdown(float seconds, T_CALLBACK callback);
	bool run_countdown(void);

	bool check_and_inc_steps(void);
	void stop_and_callack(void);

	void start_overflow_timer();
	void start_compare_timer();
};

#endif
