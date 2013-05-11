/**
 * @file
 * @brief Example suite consisting of tests that require initialization.
 */

#include <stdio.h>
#include "cut.h"

typedef struct { FILE* file; } test_t;

/**
 * Test initialization function. Test is meaningless without the input file,
 * so raise an ERROR if it doesn't exist.
 */
static cut_result_t test_init(test_t* test)
{
    test->file = fopen("input-data.txt", "rt");
    CUT_ASSERT_MESSAGE(test->file != NULL, "missing \"input-data.txt\"");
    CUT_TEST_PASS();
}

/**
 * Test finalization function. It's always called, even if test_init() fails
 * (because it might have partially failed). So the state of test->variables
 * must be checked before operating on them.
 */
static void test_exit(test_t* test)
{
    if (NULL != test->file)
        fclose(test->file);
}

static cut_result_t sum_test(test_t* test)
{
    int sum = 0;
    int val = 0;
    while (fscanf(test->file, "%d", &val) == 1) {
        sum += val;
    }
    CUT_ASSERT_INT(143, sum);
    CUT_TEST_PASS();
}

static cut_result_t product_test(test_t* test)
{
    double product = 1.0;
    int val = 0;
    while (fscanf(test->file, "%d", &val) == 1) {
        product *= (double) val;
    }
    CUT_ASSERT_DOUBLE(122522400, product);
    CUT_TEST_PASS();
}

/**
 * The suite installer function calls CUT_CONFIG_SUITE() with the size of the
 * test data blob to use, the test initializer (test_init()) and test
 * finalizer (test_exit()).
 */
void example_complex(void)
{
    CUT_CONFIG_SUITE(sizeof(test_t), test_init, test_exit);
    CUT_ADD_TEST(sum_test);
    CUT_ADD_TEST(product_test);
}
