#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "version.h"
#include "util/console_util.h"
#include "commands/commands.h"

#define VERSION_SWITCH "--version"
#define COMMANDS_SWITCH "-C"

int main(int argc, char **argv)
{
    bool interactive = isatty(STDIN_FILENO);
    bool error = false;

    // Build arithmetic context, initialize commands
    init_commands();

    // Free all resources at exit
    atexit(unload_commands);

    if (argc > 1)
    {
        int arg_index = 1;
        bool enter_interactive = false;
        if (strcmp(argv[1], "--interactive") == 0 || strcmp(argv[1], "-i") == 0)
        {
            arg_index = 2;
            enter_interactive = true;
        }

        if (argc > arg_index)
        {
            // Check for help switch as first argument
            if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
            {
                printf("Usage: ccalc [--interactive] [--help | --version | --commands [N]]\n");
                return EXIT_SUCCESS;
            }

            // Check for version switch as first argument
            if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
            {
                printf(COPYRIGHT_NOTICE);
                return EXIT_SUCCESS;
            }

            // Check for commands switch as first argument and parse subsequent arguments non-interactively
            if (strcmp(argv[1], "--commands") == 0 || strcmp(argv[1], "-c") == 0)
            {
                for (int i = 2; i < argc; i++)
                {
                    if (!exec_command(argv[i])) error = true;
                }

                if (!enter_interactive)
                {
                    return error ? EXIT_FAILURE : EXIT_SUCCESS;
                }
            }
            else
            {
                report_error("Unrecognized argument: %s", argv[1]);
                return EXIT_FAILURE;
            }
        }
    }

    // If we are connected to a terminal, use readline and show whispered messages (interactive mode)
    set_interactive(interactive);
    whisper(COPYRIGHT_NOTICE
        "This program comes with ABSOLUTELY NO WARRANTY; for details type 'license'.\n"
        "This is free software, and you are welcome to redistribute it under certain conditions.\n");
    // Enter loop to read all input lines, return appropiate exit code
    if (!process_input(stdin)) error = true;
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
