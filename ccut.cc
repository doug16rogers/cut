/* Copyright (c) 2003, 2010 Doug Rogers under the terms of the MIT License. */
/* See http://www.opensource.org/licenses/mit-license.html. */
/* $Id$ */

#include "ccut.hh"

#include <stdexcept>
#include <string>

// ----------------------------------------------------------------------------
cut_result_t ccut_wrap_init(cut_init_func_t init, void* data, void* wrapper_cookie)
{
  CUT_ASSERT_NONNULL(init);
  cut_result_t result = CUT_RESULT_PASS;

  try
  {
    result = init(data);
  }
  catch (const std::runtime_error& ex)
  {
    std::string message = "std::runtime_error thrown during test initialization: ";
    message += ex.what();
    CUT_ASSERT_MESSAGE(0, message.c_str());
  }
  catch (const std::exception& ex)
  {
    std::string message = "std::exception thrown during test initialization: ";
    message += ex.what();
    CUT_ASSERT_MESSAGE(0, message.c_str());
  }
  catch (...)
  {
    CUT_ASSERT_MESSAGE(0, "Unhandled exception thrown during test initialization.");
  }

  return result;
}   // ccut_wrap_init()

// ----------------------------------------------------------------------------
void ccut_wrap_exit(cut_exit_func_t exit, void* data, void* wrapper_cookie)
{
  if (NULL == exit)
  {
    return;
  }

  try
  {
    exit(data);
  }
  // Though this does not result in marking the test as failed, it is
  // something the programmer should be concerned about, so emit a message.
  catch (const std::runtime_error& ex)
  {
    std::string message = "std::runtime_error thrown during test finalization: ";
    message += ex.what();
    cut_assert(__FILE__, __LINE__, 0, message.c_str());
  }
  catch (const std::exception& ex)
  {
    std::string message = "std::exception thrown during test finalization: ";
    message += ex.what();
    cut_assert(__FILE__, __LINE__, 0, message.c_str());
  }
  catch (...)
  {
    cut_assert(__FILE__, __LINE__, 0, "Unhandled exception thrown during test finalization.");
  }
}   // ccut_wrap_exit()

// ----------------------------------------------------------------------------
cut_result_t ccut_wrap_test(cut_test_func_t test, void* data, void* wrapper_cookie)
{
  CUT_ASSERT_NONNULL(test);
  cut_result_t result = CUT_RESULT_PASS;

  try
  {
    result = test(data);
  }
  catch (const std::runtime_error& ex)
  {
    std::string message = "std::runtime_error thrown during test: ";
    message += ex.what();
    CUT_ASSERT_MESSAGE(0, message.c_str());
  }
  catch (const std::exception& ex)
  {
    std::string message = "std::exception thrown during test: ";
    message += ex.what();
    CUT_ASSERT_MESSAGE(0, message.c_str());
  }
  catch (...)
  {
    CUT_ASSERT_MESSAGE(0, "Unhandled exception thrown during test.");
  }

  return result;
}   // ccut_wrap_test()

// ----------------------------------------------------------------------------
/**
 * Class to initialize CUT to use C++. This registers callbacks in order to
 * run each suite's init, exit and test function in a try-catch block in
 * order to mark such cases as failures.
 */
struct Initializer
{
  Initializer()
  {
    cut_set_wrapper(ccut_wrap_init, ccut_wrap_exit, ccut_wrap_test, NULL);
  }
};

/**
 * Globa variable whose constructor initializes CUT for catching exceptions
 * via the wrapper functions above.
 */
Initializer g_ccut_initializer;
