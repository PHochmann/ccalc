#include <stdlib.h>
#include <unistd.h>

#include "util/console_util.h"
#include "commands/commands.h"
#include "numerics/bigint.h"
#include "numerics/bigint_math.h"
#include "util/string_util.h"

/*
 * Scientific calculator in which you can define new functions and constants
 * https://github.com/PhilippHochmann/ccalc
 * (c) 2020 Philipp Hochmann, phil.hochmann[ät]gmail[dot]com
 */
int main(int argc, char **argv)
{
    /*size_t base = 10;
    BigInt opA = bigint_testpattern();
    //BigInt opB = bigint_testpattern();
    bigint_dump(&opA);
    //bigint_dump(&opB);
    bigint_pow(&opA, 0xFF);
    Vector builder = strbuilder_create(2);
    bigint_to_strbuilder(&opA, base, &builder);
    printf("Result: %s\n", (char*)builder.buffer);
    vec_destroy(&builder);
    bigint_dump(&opA);*/

    // Build arithmetic context, initialize commands
    init_commands();
    // Free all resources at exit
    atexit(unload_commands);
    // Parse any arguments non-interactively
    for (int i = 1; i < argc; i++) exec_command(argv[i]);
    // If we are connected to a terminal, use readline and show whispered messages (interactive mode)
    if (isatty(STDIN_FILENO)) set_interactive(true);
    // Enter loop to read all input lines, return appropiate exit code
    return process_input(stdin) ? EXIT_SUCCESS : EXIT_FAILURE;
}
