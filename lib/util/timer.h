#ifndef TIMER_INCLUDED
#define TIMER_INCLUDED

// Basic timer for operation timer, lifetime timer
struct Timer {
	unsigned long startTime;	// in seconds, use millis() / 1000
	unsigned long totalTime;	// in seconds, this maxes out at 136 years
	bool running;
};

void startTimer(Timer& t, unsigned long currTimeMillis);

void stopTimer(Timer& t, unsigned long currTimeMillis);

unsigned long elapsedTime(Timer t, unsigned long currTimeMillis);

Timer newTimer(unsigned long totalTime);

#endif