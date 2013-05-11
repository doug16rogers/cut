/**
 * @file
 * @brief Example unit test main program.
 */

#include <stdio.h>
#include "cut.h"

int main(int argc, char* argv[])
{
    cut_parse_command_line(&argc, argv);

    if (argc > 1) {
        fprintf(stderr, "unit_test: extra args not allowed.\n\n");
        cut_usage(stderr);
        return 1;
    }

    CUT_INSTALL_SUITE(example_simple);
    CUT_INSTALL_SUITE(example_complex);

    return cut_run(1);
}
