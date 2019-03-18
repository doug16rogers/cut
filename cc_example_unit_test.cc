// Copyright (c) 2003-2019 Doug Rogers under the Zero Clause BSD License.
// You are free to do whatever you want with this software. See LICENSE.txt.

/**
 * @file
 * @brief Example unit test main program for C++.
 */

#include <stdio.h>

#include <stdexcept>
#include <iostream>
#include <string>

#include "ccut.hh"

/**
 * Force a failure of the test by causing an exception when it is not
 * expected.
 */
bool g_force_failure = false;

// ----------------------------------------------------------------------------
/**
 * Throws a run-time error with the given message.
 */
void throw_runtime_error(const std::string& message) {
  throw std::runtime_error(message);
}   // throw_runtime_error()

// ----------------------------------------------------------------------------
/**
 * Simple test for exceptions.
 */
cut_result_t throw_test() {
  CUT_ASSERT_INT(6, 2 * 3);
  CUT_ASSERT_EXCEPTION(std::runtime_error, throw_runtime_error("should throw"));
  if (g_force_failure) {
    CUT_ASSERT_NO_EXCEPTION(std::runtime_error, throw_runtime_error("should throw"));
    throw_runtime_error("unprotected throw of std::runtime_error");
  }
  CUT_ASSERT_INT(16, 2 * (2 * 2) * 2);
  CUT_TEST_PASS();
}   // throw_test()

// ----------------------------------------------------------------------------
/**
 * Test suite builder.
 */
void ccut_suite() {
  CUT_ADD_TEST(throw_test);
}

// ----------------------------------------------------------------------------
/**
 * Main test program.
 */
int main(int argc, char* argv[]) {
  cut_parse_command_line(&argc, argv);

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if ((arg == "-f")|| (arg == "-force-failure")) {
      g_force_failure = true;
    } else {
      std::cerr << "unit_test: unknown option \"" << argv[i] << "\"\n"
                << "\n"
                << "  -f, -force-failure            Force a test failure due to exception.\n"
                << "\n";
      cut_usage(stderr);
      return 1;
    }
  }
  CUT_INSTALL_SUITE(ccut_suite);
  return cut_run(1);
}
