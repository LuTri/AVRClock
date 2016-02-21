#ifndef _CUSTOMTIMER_H
#define _CUSTOMTIMER_H

typedef void (*T_CALLBACK)(void);
typedef volatile unsigned char* REGISTER8BIT_PTR;
typedef volatile unsigned int* REGISTER16BIT_PTR;

class CustomTimer {
private:
	REGISTER8BIT_PTR _registers8bit[4];
	REGISTER16BIT_PTR _registers16bit[2];
	int _register_values[6];

	bool _running;

	unsigned int _act_steps;

	int _cycles;
	int _act_cycle;

	unsigned int* _all_steps;
	unsigned int* _all_overflows;

	unsigned int _processing_steps;
	unsigned int _processing_overflows;

	static CustomTimer* _INSTANCE_2ND;
	static CustomTimer* _INSTANCE_1ST;

	T_CALLBACK* _timer_callbacks;

	CustomTimer(bool is_first);

	void start_timer(void);
	void fill_reg_values(bool is_first);
public:
	static CustomTimer* GetCustomTimer(int n_timer);

	bool prepare_countdown(float* seconds, int n_cycles, T_CALLBACK* callbacks);
	bool prepare_countdown(float seconds, T_CALLBACK callback);
	bool run_countdown(void);

	bool check_and_inc_steps(void);
	void callback_and_next(void);

	void start_overflow_timer();
	void start_compare_timer();
};

#endif
