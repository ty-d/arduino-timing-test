#include <timer.h>
#include <unity.h>

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

int main(int argc, char** argv) {
    UNITY_BEGIN();
	RUN_TEST(test_timer);
    UNITY_END();

    return 0;
}