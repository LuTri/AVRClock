#ifndef _CUSTOMTIMER_H
#define _CUSTOMTIMER_H

typedef void (*T_CALLBACK)(void);

class CustomTimer {
private:
	bool _running;

	unsigned int _act_steps;

	int _cycles;
	int _act_cycle;

	unsigned int* _all_steps;
	unsigned int* _all_overflows;

	unsigned int _processing_steps;
	unsigned int _processing_overflows;

	static CustomTimer* _INSTANCE;
	T_CALLBACK* _timer_callbacks;

	CustomTimer();

	void start_timer(void);

public:
	static CustomTimer* GetCustomTimer();

	bool prepare_countdown(float* seconds, int n_cycles, T_CALLBACK* callbacks);
	bool run_countdown(void);

	bool check_and_inc_steps(void);
	void callback_and_next(void);

	void start_overflow_timer();
	void start_compare_timer();
};

#endif
