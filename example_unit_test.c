/**
 * @file
 * @brief Example unit test main program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cut.h"

extern int g_simple_force_failure;
extern int g_complex_force_failure;

static void usage(FILE* f, int exit_code) CUT_GNU_ATTRIBUTE((noexit));
static void usage(FILE* f, int exit_code) {
    fprintf(f, "\n");
    fprintf(f, "Usage: example_unit_test [test-substring...]\n");
    fprintf(f, "\n");
    fprintf(f, "  -h, -help                     Print this usage information.\n");
    fprintf(f, "  -force-simple-failure         Force failure in example_simple.\n");
    fprintf(f, "  -force-complex-failure        Force failure in example_complex.\n");
    fprintf(f, "\n");
    cut_usage(f);
    exit(exit_code);
}

int main(int argc, char* argv[])
{
    int i = 0;
    cut_parse_command_line(&argc, argv);

    CUT_INSTALL_SUITE(example_test);
    CUT_INSTALL_SUITE(example_with_init_test);

    for (i = 1; i < argc; ++i) {
        if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "-help"))) {
            usage(stdout, 0);
        } else if (0 == strcmp(argv[i], "-force-simple-failure")) {
            g_simple_force_failure = 1;
        } else if (0 == strcmp(argv[i], "-force-complex-failure")) {
            g_complex_force_failure = 1;
        } else {
            if (!cut_include_test(argv[i])) {
                fprintf(stderr, "example_unit_test: no test names match '%s'\n", argv[i]);
                fprintf(stderr, "example_unit_test: use -h for usage information\n");
                exit(1);
            }
        }
    }

    return cut_run(1);
}
