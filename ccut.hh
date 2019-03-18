// Copyright (c) 2003-2019 Doug Rogers under the Zero Clause BSD License.
// You are free to do whatever you want with this software. See LICENSE.txt.

#ifndef __ccut_hh__
#define __ccut_hh__

/**
 * @file
 * C++ Unit Testing
 *
 * Provides a C++ wrapper for the C unit testing framework. This provides for
 * catching exceptions. The wrapper is installed via a global class variable,
 * so just include the corresponding ccut.cc in your build and it will
 * automatically be configured for C++ operation.
 */
#ifndef __cplusplus
#error "This include file is for C++ only."
#endif

extern "C" {
#include "cut.h"
}

/**
 * Asserts that the exception @a _exc is thrown by @a _code.
 */
#define CUT_ASSERT_EXCEPTION(_exc,_code)                            \
  do {                                                              \
    try {                                                           \
      _code ;                                                       \
      CUT_ASSERT_MESSAGE(0, "Exception " # _exc " not thrown.");    \
    } catch (const _exc &) {                                        \
      CUT_ASSERT_MESSAGE(1, "Exception " # _exc " thrown.");        \
    }                                                               \
  } while (0)

/**
 * Asserts that the exception @a _exc is NOT thrown by @a _code.
 */
#define CUT_ASSERT_NO_EXCEPTION(_exc,_code)                         \
  do {                                                              \
    try {                                                           \
      _code ;                                                       \
      CUT_ASSERT_MESSAGE(1, "Exception " # _exc " not thrown.");    \
    } catch (const _exc &) {                                        \
      CUT_ASSERT_MESSAGE(0, "Exception " # _exc " thrown.");        \
    }                                                               \
  } while (0)

#endif
