/* Copyright (c) 2003, 2010 Doug Rogers under the terms of the MIT License. */
/* See http://www.opensource.org/licenses/mit-license.html. */
/* $Id$ */

#ifndef __cut_h__
#define __cut_h__

/**
 * @file
 * C Unit Testing
 *
 * Provides a unit testing framework for C.
 *
 * This is a simplification of my earlier version.
 *
 * An example is given in example_unit_test.c, example_simple.c, and
 * example_complex.c. See those files.
 *
 * To build and run the example, use the following:
 *
 * @code
 * gcc -o example_unit_test example_unit_test.c example_simple.c example_complex.c cut.c
 * ./example_unit_test -help
 * ./example_unit_test
 * ./example_unit_test -show-cases
 * @endcode
 *
 * The "complex" example needs "input-data.txt", which consists of the first
 * 10 Fibonacci numbers (1 1 2 3 5 8 13 21 34 55).
 */
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Test or initialization results.
 *
 * - PASS, the test was run and completely successfully.
 *
 * - FAIL, the test was run and an assertion failed.
 *
 * - SKIP, the test was not run.
 *
 * - ERROR, the test could not be run due to a failure to establish a proper
 * testing environment, usually due to a failure in the initialization
 * function.
 */
typedef enum {
  CUT_RESULT_FIRST = 0,
  CUT_RESULT_PASS = CUT_RESULT_FIRST,
  CUT_RESULT_FAIL,
  CUT_RESULT_SKIP,
  CUT_RESULT_ERROR,
  CUT_RESULT_LAST = CUT_RESULT_ERROR
} cut_result_t;

#define CUT_RESULT_COUNT (1 + CUT_RESULT_LAST - CUT_RESULT_FIRST)

/**
 * Text name of result.
 */
extern const char* cut_result_name[CUT_RESULT_COUNT];

/*
 * Types for various callbacks.
 */
typedef void         (*cut_install_func_t)(void);
typedef cut_result_t (*cut_init_func_t)(void* data);
typedef void         (*cut_exit_func_t)(void* data);
typedef cut_result_t (*cut_test_func_t)(void* data);

cut_result_t cut_install_suite(const char* name, cut_install_func_t suite_install);
cut_result_t cut_config_suite(size_t size, cut_init_func_t test_init, cut_exit_func_t test_exit);
cut_result_t cut_add_test(const char* test_name, cut_test_func_t test_func);

/**
 * In your main test program (that is, not a particular test suite), use this
 * macro to install a test suite with the given @a _name. Supply the name of
 * the suite without quotes. This results in a call to name() which must have
 * external linkage.
 */
#define CUT_INSTALL_SUITE(_name)       \
  do {                                 \
    extern void _name();               \
    cut_install_suite(# _name, _name); \
  } while (0)

/**
 * Should be used in the test suite's installer function (see
 * CUT_INSTALL_SUITE()). This adds full data and initialization/finalization
 * functionality for the current test suite.  This is not necessary for
 * simple tests that do not require pre-test or post-test processing.
 *
 * Before running each test in a suite, a data buffer of @a _size bytes will
 * be allocated and zeroed, then passed to the test suite's @a _init()
 * function. This allows a suite to pre-initialize data for each test. After
 * testing (or failure of @a _init()), the data buffer is passed to @a
 * _exit() for finalization before the buffer is zeroed for the next test.
 */
#define CUT_CONFIG_SUITE(_size,_init,_exit) cut_config_suite(_size, (cut_init_func_t) _init, (cut_exit_func_t) _exit)

/**
 * Called from the suite's installer, this macro adds a test for the function
 * with the given @a _name.
 */
#define CUT_ADD_TEST(_name)  cut_add_test( # _name, (cut_test_func_t) _name)

/**
 * Processes command line arguments for cut-specific settings. If an error is
 * found in a cut-specific setting then a message is printed to stderr and
 * CUT_RESULT_FAIL is returned. The cut-specific arguments are removed as
 * they are consumed.
 *
 * See cut_usage() in cut.c for the available options.
 *
 * @return CUT_RESULT_PASS on success, CUT_RESULT_FAIL if an error was detected.
 */
cut_result_t cut_parse_command_line(int* argc, char* argv[]);

/**
 * Prints cut-specific usage information to the given @a file, or to stdout
 * if @a file is NULL.
 */
void cut_usage(FILE* file);

/**
 * Run the entire suite - all tests that are currently enabled.
 *
 * @param print_summary - if non-zero, a summary will be printed to stdout.
 *
 * @return a cut_result_t code, the first of each of these conditions:
 * - CUT_RESULT_ERROR if any of the tests reported an error condition.
 * - CUT_RESULT_FAIL if any of the tests reported a failure.
 * - CUT_RESULT_SKIP if *all* of the tests were skipped.
 * - CUT_RESULT_PASS if at least one test passed and there were no errors or
 * failures.
 */
cut_result_t cut_run(int print_summary);

/**
 * Flags for results.
 */
#define CUT_RESULT_FLAG(_res)   ((_res > CUT_RESULT_LAST) ? 0 : (1 << _res))

#define CUT_FLAG_PASS   CUT_RESULT_FLAG(CUT_RESULT_PASS)
#define CUT_FLAG_FAIL   CUT_RESULT_FLAG(CUT_RESULT_FAIL)
#define CUT_FLAG_SKIP   CUT_RESULT_FLAG(CUT_RESULT_SKIP)
#define CUT_FLAG_ERROR  CUT_RESULT_FLAG(CUT_RESULT_ERROR)

#define CUT_FLAG_ALL    ((1 << CUT_RESULT_COUNT) - 1)

/**
 * Bit flags indicating whether test names should be printed based on the
 * test result. Use CUT_RESULT_FLAG(_res) to set or clear the bits.
 */
extern int cut_print_test_flags;

/**
 * Bit flags indicating whether individual test cases (assertions) should be
 * printed based on the case result. Use CUT_RESULT_FLAG(_res) to set or
 * clear the bits.
 */
extern int cut_print_case_flags;

/**
 * Registers the result of an assertion.
 * All of the other assertion functions and macros end up calling this.
 */
cut_result_t cut_assertion_result(const char*  file,
                                  int          line,
                                  cut_result_t result,
                                  const char*  message);

/**
 * Makes an assertion for the currently running test or test_init() function.
 * All of the other assertion functions and macros end up calling this.
 */
cut_result_t cut_assert(const char* file,
                        int         line,
                        int         condition,
                        const char* message);

/**
 * Allows freely formatted printing of the message associated with the
 * assertion.
 */
cut_result_t cut_assertf(const char* file,
                         int         line,
                         int         condition,
                         const char* format,
                         ...) __attribute__((format(printf,4,5)));

/**
 * Sets the type of an int, which should be large enough to hold a pointer,
 * too.
 */
typedef long long cut_int_t;

cut_result_t cut_assert_pointer(const char* file, int line, const void* proper, const void* actual);
cut_result_t cut_assert_int_in(const char* file, int line, cut_int_t proper_lo, cut_int_t proper_hi, cut_int_t actual);
cut_result_t cut_assert_double_in(const char* file, int line, double proper_lo, double proper_hi, double actual);
cut_result_t cut_assert_string(const char* file, int line, const char* proper, const char* actual);
cut_result_t cut_assert_memory(const char* file, int line, const void* proper, const void* actual, size_t n);

/**
 * Default epsilon value for a comparision of doubles. The following
 * two assertions are made:
 *
 *   actual >= proper * (1.0 - epsilon)
 *   actual <= proper * (1.0 + epsilon)
 *
 * Both must be true for the assertion to succeed.
 */
#define CUT_EPSILON   0.000001

/**
 * Examines the result of running @a _code and return
 */
#define CUT_RETURN(_code)             \
  do {                                \
    cut_result_t result = _code ;     \
    if (CUT_RESULT_PASS != result) {  \
      return result;                  \
    }                                 \
  } while (0)

/**
 * These are arguments that are passed to all assertions.
 */
#define CUT_WHERE_ARGS    __FILE__, __LINE__

#define CUT_ASSERT_MESSAGE(_cond,_msg)     CUT_RETURN(cut_assert(CUT_WHERE_ARGS, ( _cond ), _msg ))
#define CUT_ASSERT(_cond)                  CUT_ASSERT_MESSAGE(_cond, # _cond )
#define CUT_ASSERT_INT_IN(_lo,_hi,_a)      CUT_RETURN(cut_assert_int_in(CUT_WHERE_ARGS, (cut_int_t) (_lo), (cut_int_t) (_hi), (cut_int_t) (_a)))
#define CUT_ASSERT_INT(_p,_a)              CUT_ASSERT_INT_IN((_p), (_p), (_a))
#define CUT_ASSERT_POINTER(_p,_a)          CUT_RETURN(cut_assert_pointer(CUT_WHERE_ARGS, (_p), (_a)))
#define CUT_ASSERT_DOUBLE_IN(_lo,_hi,_a)   CUT_RETURN(cut_assert_double_in(CUT_WHERE_ARGS, (_lo), (_hi), (_a)))
#define CUT_ASSERT_DOUBLE_NEAR(_p,_a,_eps) CUT_ASSERT_DOUBLE_IN((_p) * (1.0 - (_eps)), (_p) * (1.0 + (_eps)), (_a))
#define CUT_ASSERT_DOUBLE(_p,_a)           CUT_ASSERT_DOUBLE_NEAR((_p), (_a), CUT_EPSILON)
#define CUT_ASSERT_DOUBLE_EXACT(_p,_a)     CUT_ASSERT_DOUBLE_NEAR((_p), (_a), 0.0)
#define CUT_ASSERT_STRING(_p,_a)           CUT_RETURN(cut_assert_string(CUT_WHERE_ARGS, (_p), (_a)))
#define CUT_ASSERT_MEMORY(_p,_a,_n)        CUT_RETURN(cut_assert_memory(CUT_WHERE_ARGS, (_p), (_a), (_n)))
#define CUT_ASSERT_NULL(_a)                CUT_ASSERT(((_a) == NULL))
#define CUT_ASSERT_NONNULL(_a)             CUT_ASSERT(((_a) != NULL))

/**
 * Use this to end the current test with the given result (just the short
 * result name, not with CUT_RESULT_).
 */
#define CUT_TEST_END(_res)                                              \
  do {                                                                  \
  return cut_assertion_result(__FILE__, __LINE__, CUT_RESULT_ ## _res, "end test: " # _res); \
  } while (0)

/**
 * Use this to end a test with a passing result.
 */
#define CUT_TEST_PASS()    CUT_TEST_END(PASS)

/**
 * Use this to skip the current test.
 */
#define CUT_TEST_SKIP()    CUT_TEST_END(SKIP)

/**
 * Use this to end a test with a failure result.
 */
#define CUT_TEST_FAIL()    CUT_TEST_END(FAIL)

#ifdef __cplusplus
}
#endif

#endif
