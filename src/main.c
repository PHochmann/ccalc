#include <stdlib.h>
#include <unistd.h>

#include "util/console_util.h"
#include "commands/commands.h"

/*
 * Scientific calculator in which you can define new functions and constants
 * https://github.com/PhilippHochmann/ccalc
 * (c) 2020 Philipp Hochmann, phil.hochmann[at]gmail[dot]com
 */
int main(int argc, char **argv)
{
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
