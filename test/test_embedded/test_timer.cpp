#include <Arduino.h>
#include <unity.h>
#include <timer.h>

void test_timer() {
	Timer t = newTimer(0);

	startTimer(t, 5000);
	TEST_ASSERT_EQUAL(5, t.startTime);

    // timer has 1 second resolution
	startTimer(t, 5100);
	TEST_ASSERT_EQUAL(5, t.startTime);

	startTimer(t, 5900);
	TEST_ASSERT_EQUAL(5, t.startTime);

    // 3 seconds have passed
	TEST_ASSERT_EQUAL(3, elapsedTime(t, 8000));

    stopTimer(t, 9000);

	TEST_ASSERT_EQUAL(4, elapsedTime(t, 13000));

    startTimer(t, 14000);

	TEST_ASSERT_EQUAL(5, elapsedTime(t, 15000));
}


void setup() {
	// says to do this in the calculator unit test example: https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp 
	delay(2000);

	UNITY_BEGIN();
	RUN_TEST(test_timer);
	UNITY_END();
}

void loop() {
	delay(1000);
}