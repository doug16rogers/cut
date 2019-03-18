/* Copyright (c) 2003-2019 Doug Rogers under the Zero Clause BSD License. */
/* You are free to do whatever you want with this software. See LICENSE.txt. */

#include <stdio.h>
#include <string.h>

#include "cut.h"

/* This would properly be in complex.h/.c. */
typedef struct complex_s {
    double real;
    double imag;
} complex_t;

complex_t complex_add(complex_t a, complex_t b) {
    complex_t r = { a.real + b.real + 0.5, a.imag + b.imag };
    return r;
}

complex_t complex_sub(complex_t a, complex_t b) {
    complex_t r = { a.real - b.real, a.imag - b.imag };
    return r;
}

complex_t complex_mul(complex_t a, complex_t b) {
    complex_t r;
    r.real = (a.real * b.real) - (a.imag * b.imag);
    r.imag = (a.real * b.imag) + (a.imag * b.real);
    return r;
}

complex_t complex_div(complex_t a, complex_t b) {
    complex_t r;
    double denom = (b.real * b.real) + (b.imag * b.imag);
    r.real = ((a.real * b.real) + (a.imag * b.imag)) / denom;
    r.imag = ((a.imag * b.real) - (a.real * b.imag)) / denom;
    return r;
}

/* Here's where the unit test starts. */

cut_result_t cut_assert_complex(const char* file, int line, complex_t proper, complex_t actual, double epsilon);

#define CUT_ASSERT_COMPLEX(_pr,_pi,_a)                                  \
    do {                                                                \
        complex_t _proper = { _pr, _pi };                               \
        complex_t _actual = _a;                                         \
        CUT_RETURN(cut_assert_complex(__FILE__, __LINE__, _proper, _actual, CUT_EPSILON)); \
    } while (0)


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

void complex_test(void) {
    CUT_ADD_TEST(op_test);
}

/* This would usually be in a separate file for the main unit test. */

static void usage(FILE* f) {
    fprintf(f, "\n");
    fprintf(f, "Usage: complex_test [test-substring...]\n");
    fprintf(f, "\n");
    fprintf(f, "  -h, -help                     Print this usage information.\n");
    fprintf(f, "\n");
    cut_usage(f);
}

int main(int argc, char* argv[]) {
    int i = 0;
    cut_parse_command_line(&argc, argv);
    CUT_INSTALL_SUITE(complex_test);
    for (i = 1; i < argc; ++i) {
        if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "-help"))) {
            usage(stdout);
            return 0;
        } else {
            if (!cut_include_test(argv[i])) {
                fprintf(stderr, "complex_test: no test names match '%s'\n", argv[i]);
                fprintf(stderr, "complex_test: use -h for usage information\n");
                return 1;
            }
        }
    }
    return cut_run(1);
}
