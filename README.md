C Unit Test Library
===================

Cut is a C unit testing framework that intends to be easy to use and easy to
run. It has a few nice properties:

- It is simple to write tests.

- Its output is easy to understand.

- It is extendable.

- It is comprised of just two files - cut.h and cut.c.

There are examples provided.

Learning to Use cut
-------------------

The best way to learn how to use Cut is to take a look at example_unit_test.c
and its test suites in example_test.c and example_with_init_test.c.

example_unit_test.c provides command line argument parsing and printing of
usage information. It allows Cut to print and handle all the Cut-specific
options. It installs the two test suites. Installing consists of calling the
test suite's registration function, which in turn registers each test within
the suite. Finally, the set of test suites is run using cut_run(), which
returns a value suitable for use as a program exit code based on the
pass/fail result of the test.

example_test.c shows how to add tests that do not require any setup or
teardown steps before and after the tests. It demonstrates a number of
assertion types. Note how the registration function, example_test(), does
not need to perform a configuration step - by default no setup/teardown
routines will be used.

example_with_init_test.c shows how to create tests that require setup and
teardown steps around each test. CUT_CONFIG_SUITE() provides for specifying
the size of the data buffer to be passed to the test and its setup and
teardown functions. The buffer contents are zeroed and it is passed as a void
pointer to the setup function (test_init() in example_with_init_test.c),
which can then put items into the buffer as required. The test is then called
with the same pointer. Finally, the teardown (test_exit()) function is
called, also passing the buffer pointer. Note that the teardown function is
always called, even if the setup function fails. This allows for cleanup of
partially acquired resources.

Building the Examples
---------------------

To build the examples on Unix-like systems, run 'make':

```
    $ make
    Makefile:82: Makefile.depend: No such file or directory
    gcc -M   example_unit_test.c | sed 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g' > example_unit_test.d
    gcc -M   example_simple.c | sed 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g' > example_simple.d
    gcc -M   example_complex.c | sed 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g' > example_complex.d
    gcc -M   cut.c | sed 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g' > cut.d
    g++ -M   cc_example_unit_test.cc | sed 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g' > cc_example_unit_test.d
    g++ -M   ccut.cc | sed 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g' > ccut.d
    Updating Makefile.depend.
    gcc -o example_unit_test.o -g -Wall -Werror    -c example_unit_test.c
    gcc -o example_simple.o -g -Wall -Werror    -c example_simple.c
    gcc -o example_complex.o -g -Wall -Werror    -c example_complex.c
    gcc -o cut.o -g -Wall -Werror    -c cut.c
    gcc -g -Wall -Werror    -o example_unit_test example_unit_test.o example_simple.o example_complex.o cut.o  
    g++ -o cc_example_unit_test.o  -c cc_example_unit_test.cc
    g++ -o ccut.o  -c ccut.cc
    g++  -o cc_example_unit_test cc_example_unit_test.o ccut.o cut.o  
```

On Windows, bring up vs/cut.sln in Visual Studio 2015 and Build the
solution. It will build both example_unit_test.exe and
cc_example_unit_test.exe in the appropriate target directory (Debug, Release,
x64/Debug, or x64/Release).

Running the Examples
--------------------

The C example produces the following output:

```
    $ ./example_unit_test
    17:48:50 example_simple.one ................................ PASS  00:00.000014
    17:48:50 example_simple.two ................................ PASS  00:00.000000
    17:48:50 example_simple.three_internal_skip ................ SKIP  00:00.000000
    17:48:50 example_simple.four ............................... PASS  00:01.258231
    17:48:51 example_simple.fail_me ............................ PASS  00:00.000003
    17:48:51 example_complex.sum ............................... PASS  00:00.000097
    17:48:51 example_complex.product ........................... PASS  00:00.000042

                    PASS    FAIL    SKIP   ERROR    Total
    Assertions        12       0       1       0       13
    Tests              6       0       1       0        7
    Result: PASS
```

Notice how each test shows the local time that it started, the suite and test
names (with any trailing "_test" removed), the result, and the elapsed time
for the test. cut is not itself multithreaded and it does not store times
anywhere so it cannot show a percent complete as the test is running, but if
you've run the test before then you can judge how long you'll need to wait in
any subsequent runs.

The meanings of PASS, FAIL, SKIP, and ERROR are explained in the section
below, Basics of the Framework.

The C++ example does not include very many tests; it consists of an
exception-test:

```
    $ ./cc_example_unit_test 
    17:50:34 ccut_suite.throw .................................. PASS  00:00.000090

                    PASS    FAIL    SKIP   ERROR    Total
    Assertions         5       0       0       0        5
    Tests              1       0       0       0        1
    Result: PASS
```

Note that C++ support is minimal - it does not yet wrap the test suites in a
C++ object, so basically you write your tests using the C style.

Running Individual Tests and Other Options
------------------------------------------

cut can allow you to run individual tests, or tests that match a particular
pattern. The pattern is not a regular expression; it's a simple substring
search. If it matches then it runs that test. Below is a run of the example
unit test that runs all tests that contain ".t" or "prod":

```
    $ ./example_unit_test  .t prod
    14:49:56 example_simple.one ................................ SKIP  00:00.000000
    14:49:56 example_simple.two ................................ PASS  00:00.000001
    14:49:56 example_simple.three_internal_skip ................ SKIP  00:00.000000
    14:49:56 example_simple.four ............................... SKIP  00:00.000000
    14:49:56 example_simple.fail_me ............................ SKIP  00:00.000000
    14:49:56 example_complex.sum ............................... SKIP  00:00.000000
    14:49:56 example_complex.product ........................... PASS  00:00.000043

                    PASS    FAIL    SKIP   ERROR    Total
    Assertions         4       0       1       0        5
    Tests              2       0       5       0        7
    Result: PASS
```

Note that "example_simple.three_internal_skip" was also run since it matched
".t", but the result of running the test was SKIP - the test is not yet
implemented.

A few other options are available, such as listing all assertions, not just
the ones that fail. Run 'example_unit_test -h' to see the options.

Basics of the Framework
-----------------------

The framework provides three layers of testing:

- Assertion - a comparison of a single value or operation produced by the
  unit under test (UUT) with the value deemed to be proper by the test
  creator. An assertion might be an exact comparison or a range comparison,
  etc.

- Test - set of assertions about the operation of a chunk of code as it moves
  through multiple phases of operation. If any assertion fails, the test
  aborts immediately under the assumption that subsequent assertions will
  also fail due to the conditions left by the failure of the previous
  operation.

- Suite - a collection of tests that operate over different aspects of the
  same UUT. When a setup and teardown procedure are needed they are applied
  to each test within the corresponding suite.

The full test name consists of the suite name and the individual test name,
each with any trailing "[_]test" removed. So if your code adds a suite using
CUT_INSTALL_SUITE(funtest) that includes a test via CUT_ADD_TEST(guy_test),
the resulting test name would be "fun.guy".

The results of an assertion or test can be PASS, FAIL, SKIP, or ERROR. Here's
what those mean:

- PASS - the assertion or test was performed and all assertions were
  determined to be true.

- FAIL - the assertion or test was performed and at least one assertion was
  determined to be falls.

- SKIP - the test was explicitly skipped internally (by returning
  CUT_RESULT_SKIP) or implicitly by selecting a subset of tests to run via
  cut_include_test(). This is useful for enumerating a series of tests for a
  UUT and slowly adding functionality.

- ERROR - the preconditions for a test could not be established. This happens
  when a test's setup function cannot allocate resources or cannot otherwise
  create the state necessary for the test to be run.

Currently the test or test setup (init) function will immediately return when
an assertion fails. All of the CUT_ASSERT_xxx() assertions eventually call
cut_assert(). See the complete list in cut.h.

Creating Custom Assertions
--------------------------

Some of the longer macros - the ones that begin CUT_FL_ASSERT_xxx() and
CUT_FLM_ASSERT_xxx() - are provided to allow custom assertions. For example:

```C
    /* These might be in a header file: */

    cut_result_t cut_assert_complex(const char* file, int line, complex_t proper, complex_t actual, double epsilon);

    #define CUT_ASSERT_COMPLEX(_pr,_pi,_a)                              \
      do {                                                              \
        complex_t _proper = { _pr, _pi };                               \
        complex_t _actual = _a;                                         \
        CUT_RETURN(cut_assert_complex(__FILE__, __LINE__, _proper, _actual, CUT_EPSILON)); \
      } while (0)

    /* And the implementations: */

    cut_result_t cut_assert_complex(const char* file, int line, complex_t proper, complex_t actual, double epsilon) {
      CUT_FLM_ASSERT_DOUBLE_NEAR(file, line, proper.real, actual.real, epsilon, "real");
      CUT_FLM_ASSERT_DOUBLE_NEAR(file, line, proper.imag, actual.imag, epsilon, "imag");
      return CUT_RESULT_PASS;
    }

    static cut_result_t op_test(void) {
      complex_t a = { -1, 3 };
      complex_t b = { 4, 0 };
      CUT_ASSERT_COMPLEX(3, 3, complex_add(a, b));
      CUT_ASSERT_COMPLEX(3, 3, complex_add(b, a));
      CUT_ASSERT_COMPLEX(-5, 3, complex_sub(a, b));
      CUT_ASSERT_COMPLEX(5, -3, complex_sub(b, a));
      CUT_ASSERT_COMPLEX(-4, 12, complex_mul(a, b));
      CUT_ASSERT_COMPLEX(-4, 12, complex_mul(b, a));
      CUT_ASSERT_COMPLEX(-0.25, 0.75, complex_div(a, b));
      CUT_ASSERT_COMPLEX(-0.4, -1.2, complex_div(b, a));
      CUT_TEST_PASS();
    }
```

If complex_add() has a bug in it - off by 0.5 in the real component, the test
output would look like:

```
    14:37:49 complex.op ........................................ 
    complex_test.c:58: FAIL  
      Lower:  2.999997000000000E+00 (3)
      Actual: 3.500000000000000E+00 (3.5) (> Upper) - real
      Upper:  3.000003000000000E+00 (3)
    14:37:49 complex.op ........................................ FAIL  00:00.000020
```

Use of CUT_FLM_ASSERT_DOUBLE_NEAR() allows the test assertion function to
label the field that caused the failure. Without that ability it would be
unclear which field was incorrect since the proper value is 3 for both of
them.

Using cut with C++
------------------

Wrappers are provided for use with C++. See ccut.hh and
cc_example_unit_test.cc. Currently the wrappers provide nothing more than
assertions that may be used to capture expected and unexpected exceptions.

A reasonable to-do item for cut would be to create a test suite class that
performs all the registrations automatically like Google's gtest or CppUnit,
but using cut as the backend in order to maintain the same look-and-feel as
the C version.

Building the Documentation
--------------------------

This file and the source code itself are probably enough to understand how to
use cut, but API documentation is also available using doxygen, which must be
installed separately. Use the following command to build the API
documentation:

```
    $ make doc
```

To-Do Items
-----------

The following are TODO items:

- Allow for continuation on failure?

- Create a C++ class wrapper for test suites.

- Automatically add C++ test suites during run-time initialization.

End
