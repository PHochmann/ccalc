#include <stdlib.h>
#include <unistd.h>

#include "util/console_util.h"
#include "commands/commands.h"

int main(int argc, char **argv)
{
    bool interactive = isatty(STDIN_FILENO);

    if (interactive)
    {
        printf("ccalc  Copyright (C) 2020  Philipp Hochmann\n"
            "This program comes with ABSOLUTELY NO WARRANTY; for details type `license'.\n"
            "This is free software, and you are welcome to redistribute it under certain conditions.\n\n");
    }

    // Build arithmetic context, initialize commands
    init_commands();
    // Free all resources at exit
    atexit(unload_commands);
    // Parse any arguments non-interactively
    for (int i = 1; i < argc; i++) exec_command(argv[i]);
    // If we are connected to a terminal, use readline and show whispered messages (interactive mode)
    set_interactive(interactive);
    // Enter loop to read all input lines, return appropiate exit code
    return process_input(stdin) ? EXIT_SUCCESS : EXIT_FAILURE;
}
