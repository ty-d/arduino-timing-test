#include <fsm.h>
#include <unity.h>

void test_add(void) {
	TEST_ASSERT_EQUAL(11, add(5, 6));
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_add);
    UNITY_END();

    return 0;
}