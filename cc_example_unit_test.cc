/**
 * @file
 * @brief Example unit test main program for C++.
 */

#include <stdio.h>
#include <stdexcept>

#include "ccut.hh"

// ----------------------------------------------------------------------------
/**
 * Throws a run-time error with the given message.
 */
void throw_runtime_error(const std::string& message)
{
  throw std::runtime_error(message);
}   // throw_runtime_error()

// ----------------------------------------------------------------------------
/**
 * Simple test for exceptions.
 */
cut_result_t throw_test()
{
  CUT_ASSERT_INT(6, 2 * 3);
  CUT_ASSERT_EXCEPTION(std::runtime_error, throw_runtime_error("should throw"));
  // These will cause the test to fail:
  // CUT_ASSERT_NO_EXCEPTION(std::runtime_error, throw_runtime_error("should throw"));
  // throw_runtime_error("unprotected throw of std::runtime_error");
  CUT_ASSERT_INT(16, 2 * (2 * 2) * 2);
  CUT_TEST_PASS();
}   // throw_test()

// ----------------------------------------------------------------------------
/**
 * Test suite builder.
 */
void ccut_suite()
{
  CUT_ADD_TEST(throw_test);
}

// ----------------------------------------------------------------------------
/**
 * Main test program.
 */
int main(int argc, char* argv[])
{
  cut_parse_command_line(&argc, argv);

  if (argc > 1) {
    fprintf(stderr, "unit_test: extra args not allowed.\n\n");
    cut_usage(stderr);
    return 1;
  }

  CUT_INSTALL_SUITE(ccut_suite);
  return cut_run(1);
}
