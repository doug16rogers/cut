/* Copyright (c) 2003, 2010 Doug Rogers under the terms of the MIT License. */
/* See http://www.opensource.org/licenses/mit-license.html. */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "cut.h"

/**
 * Text name of result.
 */
const char* cut_result_name[CUT_RESULT_COUNT] =
{
  "PASS",
  "FAIL",
  "SKIP",
  "ERROR",
};

/**
 * Bit flags indicating whether test names should be printed based on the
 * test result. Use CUT_RESULT_FLAG(_res) to set or clear the bits.
 */
int cut_print_test_flags = CUT_FLAG_PASS | CUT_FLAG_FAIL | CUT_FLAG_ERROR;

/**
 * Bit flags indicating whether individual test cases (assertions) should be
 * printed based on the case result. Use CUT_RESULT_FLAG(_res) to set or
 * clear the bits.
 */
int cut_print_case_flags = CUT_FLAG_FAIL | CUT_FLAG_ERROR;

/**
 * Maximum length of a suite or test name. Note that the test name includes
 * the suite name, so suites must actually be shorter.
 */
#define CUT_NAME_LEN_MAX  0x100

typedef struct cut_test_s  cut_test_t;
typedef struct cut_suite_s cut_suite_t;

/**
 * Test type.
 */
struct cut_test_s
{
  /**
   * The name includes the suite name ("suite.test").
   */
  char name[CUT_NAME_LEN_MAX];

  /**
   * Test function.
   */
  cut_test_func_t func;

  /**
   * Parent suite.
   */
  cut_suite_t* suite;

  /**
   * Next test in list, or NULL if the current test is the last.
   */
  cut_test_t* next;

};  /* struct cut_test_s */

/**
 * Suite type.
 */
struct cut_suite_s
{
  /**
   * The name includes the suite name ("suite.test").
   */
  char name[CUT_NAME_LEN_MAX];

  /**
   * Number of data bytes to send.
   */
  size_t size;

  /**
   * Data pointer. This is allocated up front.
   */
  void* data;

  /**
   * Initialization function for each test in suite.
   */
  cut_init_func_t init;

  /**
   * Finalization function for each test in suite.
   */
  cut_exit_func_t exit;

  /**
   * Linked list of tests.
   */
  cut_test_t* test;

  /**
   * Next suite in list.
   */
  cut_suite_t* next;
};   /* struct cut_suite_s */

/**
 * Global cut object, just in case I want to objectify it some day. The whole
 * point of this exercise was to simplify my existing unit test framework to
 * get rid of all that, but I can't seem to resist this concession.
 */
typedef struct cut_s
{
  /**
   * List of suites.
   */
  cut_suite_t* suite;

  /**
   * Suite currently in use.
   */
  cut_suite_t* active_suite;

  /**
   * Test currently in use.
   */
  cut_test_t* active_test;

  /**
   * Number of assertions made with each type of result.
   */
  unsigned int assertions[CUT_RESULT_COUNT];

  /**
   * Number of tests finished with each type of result.
   */
  unsigned int tests[CUT_RESULT_COUNT];

  /**
   * Set when the test name is printed in order to track when to add
   * newlines.
   */
  int test_name_hanging;
} cut_t;

/**
 * Global (singleton) C unit test object.
 */
static cut_t g_cut_info =
{
  .suite        = NULL,
  .active_suite = NULL,
  .assertions   = { 0, 0, 0, 0 },
  .tests        = { 0, 0, 0, 0 },
};   /* g_cut_info */

/**
 * Pointer to global C unit test object.
 */
static cut_t* g_cut = &g_cut_info;

/**
 * Maximum length of character image function.
 */
#define CHAR_IMAGE_MAX_LEN 8

/**
 * Maximum length of a string to be printed when a mismatch is detected. This
 * should be used by all strings passed to printable_diff_string().
 */
#define PRINTABLE_DIFF_STRING_MAX_LEN  64

/* ------------------------------------------------------------------------- */
/**
 * @param c - the character whose image is sought.
 *
 * @param image - buffer to hold the image, or NULL for a shared buffer. This
 * must point to a buffer of at least CHAR_IMAGE_MAX_LEN characters.
 *
 * @return a printable image of character @a c.
 */
static const char* char_image(char c, char* image)
{
  static char shared_image[CHAR_IMAGE_MAX_LEN] = "";

  image = (NULL == image) ? shared_image : image;

  if (isprint(c))
  {
    image[0] = '\'';
    image[1] = c;
    image[2] = '\'';
    image[3] = 0;
    return image;
  }

  switch (c)
  {
  case '\t': strncpy(image, "'\\t'", CHAR_IMAGE_MAX_LEN); break;
  case '\r': strncpy(image, "'\\r'", CHAR_IMAGE_MAX_LEN); break;
  case '\n': strncpy(image, "'\\n'", CHAR_IMAGE_MAX_LEN); break;

  default:
    if (c < 10) snprintf(image, CHAR_IMAGE_MAX_LEN - 1, "'\\%u'", (unsigned) (unsigned char) c);
    else        snprintf(image, CHAR_IMAGE_MAX_LEN - 1, "'\\x%02X'", (unsigned) (unsigned char) c);
  }

  image[CHAR_IMAGE_MAX_LEN-1] = 0;
  return image;
}   /* char_image() */

/* ------------------------------------------------------------------------- */
static char* printable_diff_string(char* printable, const char* src, size_t len, size_t diff_index)
{
  const size_t half_len = (PRINTABLE_DIFF_STRING_MAX_LEN / 2) - 2;   /* Allow for two dots on either side. */

  assert(src && printable);

  if (len < PRINTABLE_DIFF_STRING_MAX_LEN)
  {
    /*
     * If there's no need to truncate.
     */
    strncpy(printable, src, PRINTABLE_DIFF_STRING_MAX_LEN-1);
  }
  else if (diff_index > half_len)
  {
    /*
     * If the difference appears more than halfway through the buffer, use a
     * leading ellipsis.
     */
    printable[0] = '.';
    printable[2] = '.';
    strncpy(&printable[2], &src[diff_index - half_len], PRINTABLE_DIFF_STRING_MAX_LEN-1-2);

    if (len > (diff_index + half_len))
    {
      /*
       * If a trailing ellipsis is needed, too.
       */
      printable[PRINTABLE_DIFF_STRING_MAX_LEN-1-2] = '.';
      printable[PRINTABLE_DIFF_STRING_MAX_LEN-1-1] = '.';
    }
  }
  else   /* else the difference occurs early in a long string... */
  {
    strncpy(printable, src, PRINTABLE_DIFF_STRING_MAX_LEN-1);
    printable[PRINTABLE_DIFF_STRING_MAX_LEN-1-2] = '.';
    printable[PRINTABLE_DIFF_STRING_MAX_LEN-1-1] = '.';
  }

  printable[PRINTABLE_DIFF_STRING_MAX_LEN-1] = 0;
  return printable;
}   /* printable_diff_string() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_install_suite(const char* name, cut_install_func_t suite_install)
{
  cut_suite_t* suite = NULL;
  int result = CUT_RESULT_PASS;

  assert(NULL != name);
  assert(NULL != suite_install);

  suite = (cut_suite_t*) malloc(sizeof(*suite));

  if (NULL == suite)
  {
    return CUT_RESULT_FAIL;
  }

  memset(suite, 0, sizeof(*suite));
  strncpy(suite->name, name, CUT_NAME_LEN_MAX);
  suite->name[CUT_NAME_LEN_MAX-1] = 0;
  g_cut->active_suite = suite;

  if (NULL == g_cut->suite)
  {
    g_cut->suite = suite;
  }
  else
  {
    /*
     * Yeah, this is inefficient. But it only happens on startup and I don't
     * feel like implementing a doubly-linked list to maintain insertion
     * order.
     */
    cut_suite_t* last = NULL;

    for (last = g_cut->suite; last->next != NULL; last = last->next)
    {
        /* do nothing; finding end of list. */
    }

    last->next = suite;
  }

  suite_install();
  return result;
}   /* cut_install_suite() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_config_suite(size_t size, cut_init_func_t test_init, cut_exit_func_t test_exit)
{
  cut_result_t result = CUT_RESULT_PASS;
  cut_suite_t* suite = NULL;
  assert(NULL != g_cut);
  assert(NULL != g_cut->active_suite);

  suite = g_cut->active_suite;

  if (NULL != suite->data)
  {
    free(suite->data);
    suite->data = NULL;
    suite->size = 0;
  }

  suite->init = NULL;
  suite->exit = NULL;

  if (size > 0)
  {
    suite->data = malloc(size);

    if (NULL == suite->data)
    {
      result = CUT_RESULT_FAIL;
    }
    else
    {
      suite->size = size;
      suite->init = test_init;
      suite->exit = test_exit;
    }
  }

  return result;
}   /* cut_config_suite() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_add_test(const char* test_name, cut_test_func_t test_func)
{
  cut_result_t result = CUT_RESULT_PASS;
  cut_test_t* test = NULL;
  cut_suite_t* suite = NULL;

  assert(NULL != test_name);
  assert(NULL != test_func);
  assert(NULL != g_cut);
  assert(NULL != g_cut->active_suite);

  suite = g_cut->active_suite;

  test = (cut_test_t*) malloc(sizeof(*test));

  if (NULL == test)
  {
    return CUT_RESULT_FAIL;
  }

  memset(test, 0, sizeof(*test));
  snprintf(test->name, CUT_NAME_LEN_MAX, "%s.%s", suite->name, test_name);
  test->name[CUT_NAME_LEN_MAX-1] = 0;
  test->func = test_func;
  test->suite = suite;

  if (NULL == suite->test)
  {
    suite->test = test;
  }
  else
  {
    /*
     * Yeah, this is inefficient. But it only happens on startup and I don't
     * feel like implementing a doubly-linked list to maintain insertion
     * order.
     */
    cut_test_t* last = NULL;

    for (last = suite->test; last->next != NULL; last = last->next)
    {
    }

    last->next = test;
  }

  return result;
}   /* cut_add_test() */

/* ------------------------------------------------------------------------- */
/**
 * Processes command line arguments for cut-specific settings. If an error is
 * found in a cut-specific setting, exit() is called with error code 1 and
 * the cut-specific usage information is printed (along with any registered
 * client usage information).
 *
 * See cut_usage() in cut.c for the available options.
 */
cut_result_t cut_parse_command_line(int* argc, char* argv[])
{
  int i = 0;

  assert(NULL != argc);
  assert(NULL != argv);

  for (i = 1; i < *argc; i++)
  {
    char* arg = argv[i];
    int arg_used = 1;

    assert(NULL != arg);

    if (*arg != '-')
    {
      continue;
    }

    if (*arg == '-') arg++;
    if (*arg == '-') arg++;
    if      (strcmp(arg, "show-cases"         ) == 0) cut_print_case_flags  = CUT_FLAG_ALL;
    else if (strcmp(arg, "show-pass-cases"    ) == 0) cut_print_case_flags |= CUT_FLAG_PASS;
    else if (strcmp(arg, "show-fail-cases"    ) == 0) cut_print_case_flags |= CUT_FLAG_FAIL;
    else if (strcmp(arg, "show-skip-cases"    ) == 0) cut_print_case_flags |= CUT_FLAG_SKIP;
    else if (strcmp(arg, "show-error-cases"   ) == 0) cut_print_case_flags |= CUT_FLAG_ERROR;
    else if (strcmp(arg, "show-no-cases"      ) == 0) cut_print_case_flags  = 0;
    else if (strcmp(arg, "no-show-cases"      ) == 0) cut_print_case_flags  = 0;
    else if (strcmp(arg, "no-show-pass-cases" ) == 0) cut_print_case_flags &= ~CUT_FLAG_PASS;
    else if (strcmp(arg, "no-show-fail-cases" ) == 0) cut_print_case_flags &= ~CUT_FLAG_FAIL;
    else if (strcmp(arg, "no-show-skip-cases" ) == 0) cut_print_case_flags &= ~CUT_FLAG_SKIP;
    else if (strcmp(arg, "no-show-error-cases") == 0) cut_print_case_flags &= ~CUT_FLAG_ERROR;
    else if (strcmp(arg, "show-tests"         ) == 0) cut_print_test_flags  = CUT_FLAG_ALL;
    else if (strcmp(arg, "show-pass-tests"    ) == 0) cut_print_test_flags |= CUT_FLAG_PASS;
    else if (strcmp(arg, "show-fail-tests"    ) == 0) cut_print_test_flags |= CUT_FLAG_FAIL;
    else if (strcmp(arg, "show-skip-tests"    ) == 0) cut_print_test_flags |= CUT_FLAG_SKIP;
    else if (strcmp(arg, "show-error-tests"   ) == 0) cut_print_test_flags |= CUT_FLAG_ERROR;
    else if (strcmp(arg, "show-no-tests"      ) == 0) cut_print_test_flags  = 0;
    else if (strcmp(arg, "no-show-tests"      ) == 0) cut_print_test_flags  = 0;
    else if (strcmp(arg, "no-show-pass-tests" ) == 0) cut_print_test_flags &= ~CUT_FLAG_PASS;
    else if (strcmp(arg, "no-show-fail-tests" ) == 0) cut_print_test_flags &= ~CUT_FLAG_FAIL;
    else if (strcmp(arg, "no-show-skip-tests" ) == 0) cut_print_test_flags &= ~CUT_FLAG_SKIP;
    else if (strcmp(arg, "no-show-error-tests") == 0) cut_print_test_flags &= ~CUT_FLAG_ERROR;
    else
    {
      arg_used = 0;
    }

    if (arg_used)
    {
      int j = 0;

      /*
       * Remove argument from list.
       */
      for (j = i + 1; j < *argc; j++)
      {
        argv[j-1] = argv[j];
      }

      (*argc)--;
      i--;
    }
  }   /* for each argument */

  return CUT_RESULT_PASS;
}   /* cut_parse_command_line() */

/* ------------------------------------------------------------------------- */
/**
 * Prints cut-specific usage information to the given @a file, or to stdout
 * if @a file is NULL.
 */
void cut_usage(FILE* file)
{
  fprintf(
    file,
    "  -[no-]show-cases         Do [not] show all test assertions.\n"
    "  -[no-]show-[type]-cases  Turn on showing of assertions for result <type>.\n"
    "  -show-no-cases           Same as -no-show-cases; shows no assertions.\n"
    "  -[no-]show-tests         Do [not] show all test results.\n"
    "  -[no-]show-[type]-tests  Turn on showing of test results for <type>.\n"
    "  -show-no-tests           Same as -no-show-tests; shows no test results.\n"
    "\n"
    "  <type> - Result types may be pass, fail, skip, or error.\n"
    "\n"
    );
}   /* cut_usage() */

/* ------------------------------------------------------------------------- */
/**
 * Registers the result of an assertion.
 * All of the other assertion functions and macros end up calling this.
 */
cut_result_t cut_assertion_result(const char*  file,
                                  int          line,
                                  cut_result_t result,
                                  const char*  message)
{
  assert((result >= CUT_RESULT_FIRST) && (result <= CUT_RESULT_LAST));
  assert(NULL != g_cut);

  g_cut->assertions[result]++;

  if (cut_print_case_flags & CUT_RESULT_FLAG(result))
  {
    if (g_cut->test_name_hanging)
    {
      printf("\n");
      g_cut->test_name_hanging = 0;
    }

    printf("%s:%d: %-5s %s\n", file, line, cut_result_name[result], message);
  }

  return result;
}   /* cut_assertion_result() */

/* ------------------------------------------------------------------------- */
/**
 * Makes an assertion for the currently running test or test_init() function.
 * All of the other assertion functions and macros end up calling this.
 */
cut_result_t cut_assert(const char* file,
                        int         line,
                        int         condition,
                        const char* message)
{
  cut_result_t result = CUT_RESULT_PASS;

  assert(NULL != g_cut);
  assert(NULL != g_cut->active_suite);

  if (!condition)
  {
    result = (NULL != g_cut->active_test) ? CUT_RESULT_FAIL : CUT_RESULT_ERROR;
  }

  return cut_assertion_result(file, line, result, message);
}   /* cut_assert() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_assertf(const char* file,
                         int         line,
                         int         condition,
                         const char* format,
                         ...)
{
  char message[0x100] = "";
  va_list va;
  va_start(va, format);
  vsnprintf(message, sizeof(message), format, va);
  va_end(va);
  message[sizeof(message) - 1] = 0;
  return cut_assert(file, line, condition, message);
}   /* cut_assertf() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_assert_pointer(const char* file, int line, const void* proper, const void* actual)
{
  return cut_assertf(file, line, proper == actual, "\n  Proper: @%p\n  Actual: @%p", proper, actual);
}   /* cut_assert_pointer() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_assert_int_in(const char* file, int line, long long proper_lo, long long proper_hi, long long actual)
{
#define MINGW 1
#if defined(MINGW)
  if (proper_lo == proper_hi)
  {
    return cut_assertf(file, line, proper_lo == actual,
                       "\n  Proper: %10ld (0x%08lX)\n  Actual: %10ld (0x%08lX)",
                       (long) proper_lo, (long) proper_lo, (long) actual, (long) actual);
  }
  else
  {
    return cut_assertf(file, line, (proper_lo <= actual) && (actual < proper_hi),
                       "\n  Lower:  %10ld (0x%08lX)\n  Actual: %10ld (0x%08lX)\n  Upper:  %10ld (0x%08lX)",
                       (long) proper_lo, (long) proper_lo, (long) actual, (long) actual, (long) proper_hi, (long) proper_hi);
  }
#else
  if (proper_lo == proper_hi)
  {
    return cut_assertf(file, line, proper_lo == actual,
                       "\n  Proper: %10lld (0x%08llX)\n  Actual: %10lld (0x%08llX)",
                       proper_lo, proper_lo, actual, actual);
  }
  else
  {
    return cut_assertf(file, line, (proper_lo <= actual) && (actual < proper_hi),
                       "\n  Lower:  %10lld (0x%08llX)\n  Actual: %10lld (0x%08llX)\n  Upper:  %10lld (0x%08llX)",
                       proper_lo, proper_lo, actual, actual, proper_hi, proper_hi);
  }
#endif
}   /* cut_assert_int_in() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_assert_double_in(const char* file, int line, double proper_lo, double proper_hi, double actual)
{
  if (proper_lo == proper_hi)
  {
    return cut_assertf(file, line, proper_lo == actual,
                       "\n  Proper: %18.15E (%g)\n  Actual: %18.15E (%g)",
                       proper_lo, proper_lo, actual, actual);
  }
  else
  {
    return cut_assertf(file, line, (proper_lo <= actual) && (actual < proper_hi),
                       "\n  Lower:  %18.15E (%g)\n  Actual: %18.15E (%g)\n  Upper:  %18.15E (%g)",
                       proper_lo, proper_lo, actual, actual, proper_hi, proper_hi);
  }
}   /* cut_assert_double_in() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_assert_string(const char* file, int line, const char* proper, const char* actual)
{
  size_t plen = 0;
  size_t alen = 0;
  size_t max_len = 0;
  size_t i = 0;

  /*
   * Handle the NULL cases first.
   */
  if ((NULL == proper) && (NULL == actual))
  {
    return cut_assert(file, line, 1, "\n  Proper: NULL\n  Actual: NULL");
  }
  else if (NULL == proper)
  {
    return cut_assert(file, line, 0, "\n  Proper: NULL\n  Actual: non-NULL");
  }
  else if (NULL == actual)
  {
    return cut_assert(file, line, 0, "\n  Proper: non-NULL\n  Actual: NULL");
  }

  plen = strlen(proper);
  alen = strlen(actual);

  /*
   * Eventually I need to do the pretty string slice printing, but for now I
   * just run the memory comparison, being careful to check up to the
   * readable portion of either.
   */
/*   if (alen < plen) */
/*   { */
/*     return cut_assert_memory(file, line, proper, actual, alen + 1); */
/*   } */

/*   return cut_assert_memory(file, line, proper, actual, plen); */

  max_len = plen > alen ? plen : alen;
  max_len++;   /* Include terminating NUL. */

  for (i = 0; i < max_len; i++)
  {
    if (proper[i] != actual[i])
    {
      char pcimg[CHAR_IMAGE_MAX_LEN] = "";
      char acimg[CHAR_IMAGE_MAX_LEN] = "";
      char pdiff[PRINTABLE_DIFF_STRING_MAX_LEN] = "";
      char adiff[PRINTABLE_DIFF_STRING_MAX_LEN] = "";

      return cut_assertf(file, line, 0,
                         "\n  Proper at [%d]: 0x%02X %3d %-6s \"%s\"\n  Actual at [%d]: 0x%02X %3d %-6s \"%s\")",
                         (int) i, (unsigned) proper[i], (unsigned) proper[i], char_image(proper[i], pcimg), printable_diff_string(pdiff, proper, plen, i),
                         (int) i, (unsigned) actual[i], (unsigned) actual[i], char_image(actual[i], acimg), printable_diff_string(adiff, actual, alen, i));
    }
  }

  return cut_assertf(file, line, 1, "strings of length %d (0x%02X) match", (int) plen, (int) plen);
}   /* cut_assert_string() */

/* ------------------------------------------------------------------------- */
cut_result_t cut_assert_memory(const char* file, int line, const void* proper, const void* actual, size_t n)
{
  size_t i = 0;
  const unsigned char* p = (const unsigned char*) proper;
  const unsigned char* a = (const unsigned char*) actual;

  /*
   * Handle the NULL cases first.
   */
  if ((NULL == proper) && (NULL == actual))
  {
    return cut_assert(file, line, 1, "\n  Proper: NULL\n  Actual: NULL");
  }
  else if (NULL == proper)
  {
    return cut_assert(file, line, 0, "\n  Proper: NULL\n  Actual: non-NULL");
  }
  else if (NULL == actual)
  {
    return cut_assert(file, line, 0, "\n  Proper: non-NULL\n  Actual: NULL");
  }

  for (i = 0; i < n; i++)
  {
    if (p[i] != a[i])
    {
      char pcimg[CHAR_IMAGE_MAX_LEN] = "";
      char acimg[CHAR_IMAGE_MAX_LEN] = "";

      return cut_assertf(file, line, 0,
                         "\n  Proper at [%d]: 0x%02X (%d, %s)\n  Actual at [%d]: 0x%02X (%d, %s)",
                         (int) i, p[i], p[i], char_image(p[i], pcimg),
                         (int) i, a[i], a[i], char_image(a[i], acimg));
    }
  }

  return cut_assertf(file, line, 1, "buffers of length %d (0x%02X) match", (int) n, (int) n);
}   /* cut_assert_memory() */

/* ------------------------------------------------------------------------- */
void cut_print_summary(FILE* file, cut_result_t result)
{
  size_t total_assertions = 0;
  size_t total_tests = 0;
  int i = 0;

  assert(NULL != file);
  assert((CUT_RESULT_FIRST <= result) && (result <= CUT_RESULT_LAST));

  fprintf(file, "%12s", "");

  for (i = CUT_RESULT_FIRST; i <= CUT_RESULT_LAST; i++)
  {
    fprintf(file, " %7s", cut_result_name[i]);
  }

  fprintf(file, " %8s", "Total");
  fprintf(file, "\n");

  fprintf(file, "%-12s", "Assertions");

  for (i = CUT_RESULT_FIRST; i <= CUT_RESULT_LAST; i++)
  {
    total_assertions += g_cut->assertions[i];
    fprintf(file, " %7d", g_cut->assertions[i]);
  }

  fprintf(file, " %8d\n", (int) total_assertions);

  fprintf(file, "%-12s", "Tests");

  for (i = CUT_RESULT_FIRST; i <= CUT_RESULT_LAST; i++)
  {
    total_tests += g_cut->tests[i];
    fprintf(file, " %7d", g_cut->tests[i]);
  }

  fprintf(file, " %8d\n", (int) total_tests);
  fprintf(file, "Result: %s\n", cut_result_name[result]);
}   /* cut_print_summary() */

/* ------------------------------------------------------------------------- */
static void cut_print_test_name(const char* name, struct tm* stamp)
{
  int i = 0;

  assert(g_cut);
  assert(name);
  assert(stamp);

  printf("%02u:%02u:%02u %s ", stamp->tm_hour, stamp->tm_min, stamp->tm_sec, name);
  for (i = strlen(name); i < 50; i++) printf(".");
  printf(" ");
  g_cut->test_name_hanging = 1;
  fflush(stdout);
}   /* cut_print_test_name() */

/* ------------------------------------------------------------------------- */
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
cut_result_t cut_run(int print_summary)
{
  cut_result_t run_result = CUT_RESULT_PASS;
  cut_suite_t* suite = NULL;

  assert(g_cut != NULL);

  memset(g_cut->assertions, 0, sizeof(g_cut->assertions));
  memset(g_cut->tests,      0, sizeof(g_cut->tests));

  for (suite = g_cut->suite; suite != NULL; suite = suite->next)
  {
    cut_test_t* test = NULL;

    g_cut->active_suite = suite;

    for (test = suite->test; test != NULL; test = test->next)
    {
      struct timeval start_time;
      struct timeval end_time;
      time_t         stamp_time;
      struct tm      stamp;
      long           ms = 0;
      cut_result_t   result = CUT_RESULT_PASS;

      assert(test);
      assert(test->name);

      if (suite->data)
      {
        memset(suite->data, 0, suite->size);
      }

      stamp_time = time(NULL);
#if defined(MINGW)
      stamp = *localtime(&stamp_time);
#else
      localtime_r(&stamp_time, &stamp);
#endif
      gettimeofday(&start_time, NULL);

      cut_print_test_name(test->name, &stamp);

      if (suite->init)
      {
        result = suite->init(suite->data);
      }

      /*
       * Only run test if init() succeeded.
       */
      if (CUT_RESULT_PASS == result)
      {
        if (NULL == test->func)
        {
          result = CUT_RESULT_SKIP;
        }
        else
        {
          g_cut->active_test = test;
          result = test->func(suite->data);
          g_cut->active_test = NULL;
        }
      }

      if ((result < CUT_RESULT_FIRST) ||
          (result > CUT_RESULT_LAST))
      {
        result = CUT_RESULT_ERROR;
      }

      g_cut->tests[result]++;

      /*
       * Always run the finalization function if it exists.
       */
      if (suite->exit)
      {
        suite->exit(suite->data);
      }

      /*
       * If the test name was removed due to an assertion being printed, put
       * it back.
       */
      if (!g_cut->test_name_hanging)
      {
        cut_print_test_name(test->name, &stamp);
      }

      gettimeofday(&end_time, NULL);
      ms = ((end_time.tv_sec  - start_time.tv_sec ) * 1000) +
           ((end_time.tv_usec - start_time.tv_usec) / 1000);
      printf("%-5s (%02lu:%02lu.%03lu)\n", cut_result_name[result],
             ms / (60 * 1000), (ms / 1000) % 60, ms % 1000);
      g_cut->test_name_hanging = 0;
    }   /* for each test in the suite */
  }   /* for each suite */

  if (g_cut->tests[CUT_RESULT_ERROR] > 0)
  {
    run_result = CUT_RESULT_ERROR;
  }
  else if (g_cut->tests[CUT_RESULT_FAIL] > 0)
  {
    run_result = CUT_RESULT_FAIL;
  }
  else if (g_cut->tests[CUT_RESULT_PASS] > 0)
  {
    run_result = CUT_RESULT_PASS;
  }
  else
  {
    run_result = CUT_RESULT_SKIP;
  }

  if (print_summary)
  {
    printf("\n");
    cut_print_summary(stdout, run_result);
  }

  return run_result;
}   /* cut_run() */
