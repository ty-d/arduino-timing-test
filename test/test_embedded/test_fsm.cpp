#include <Arduino.h>
#include <fsm.h>
#include <unity.h>

void test_add(void) {
	TEST_ASSERT_EQUAL(11, add(5, 6));
}

void setup() {
	// says to do this in the calculator unit test example: https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp 
	delay(2000);

	UNITY_BEGIN();
	RUN_TEST(test_add);
	UNITY_END();
}

void loop() {
	delay(1000);
}