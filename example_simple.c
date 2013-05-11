/**
 * @file
 * @brief Example suite consisting of simple tests.
 */

#include <time.h>
#include <unistd.h>
#include "cut.h"

static cut_result_t one(void)
{
    int n = 5;
    double third = 1.0 / 3.0;
    CUT_ASSERT_INT(5, n);
    CUT_ASSERT_DOUBLE(0.33333333, third);
    CUT_TEST_PASS();
}

static cut_result_t two(void)
{
    CUT_ASSERT(time(NULL) > 0);
    CUT_TEST_PASS();
}

static cut_result_t three(void)
{
    // Here's how you mark a test that's not yet ready:
    CUT_TEST_SKIP();
}

static cut_result_t four(void)
{
    // To see the time show up on the right:
    sleep(1);
    usleep(258000);
    CUT_TEST_PASS();
}

static cut_result_t fail_me(void)
{
    CUT_ASSERT_MEMORY("12345678", "123A5678", 8);
    CUT_TEST_PASS();
}

void example_simple(void)
{
    CUT_ADD_TEST(one);
    CUT_ADD_TEST(two);
    CUT_ADD_TEST(three);
    CUT_ADD_TEST(four);
    CUT_ADD_TEST(fail_me);
}
