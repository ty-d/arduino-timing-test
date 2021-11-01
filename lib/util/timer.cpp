#include <timer.h>

void startTimer(Timer& t, unsigned long currTimeMillis) {
	if (!t.running) {
		t.running = true;
		t.startTime = currTimeMillis / 1000;
	}
}

void stopTimer(Timer& t, unsigned long currTimeMillis) {
	if (t.running) {
		t.running = false;
		t.totalTime = t.totalTime + (currTimeMillis / 1000) - t.startTime;
	}
}

unsigned long elapsedTime(Timer t, unsigned long currTimeMillis) {
	if (t.running) {
		unsigned long currSec = currTimeMillis / 1000;
		t.totalTime += currSec - t.startTime;
		t.startTime = currSec;
	}
	return t.totalTime;
}

Timer newTimer(unsigned long totalTime) {
	return Timer { 0, totalTime, false };
}