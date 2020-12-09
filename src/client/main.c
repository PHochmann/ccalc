#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "version.h"
#include "util/console_util.h"
#include "commands/commands.h"

int main(int argc, char **argv)
{
    // Build arithmetic context, initialize commands
    init_commands();

    // Free all resources at exit
    atexit(unload_commands);

    bool enter_interactive = false;
    bool quiet = false;
    if (argc > 1)
    {
        int arg_index = 1;
        if (strcmp(argv[arg_index], "--interactive") == 0 || strcmp(argv[arg_index], "-i") == 0)
        {
            arg_index++;
            enter_interactive = true;
        }

        if (strcmp(argv[arg_index], "--quiet") == 0 || strcmp(argv[arg_index], "-q") == 0)
        {
            arg_index++;
            quiet = true;
        }

        if (argc > arg_index)
        {
            // Check for help switch as first argument
            if (strcmp(argv[arg_index], "--help") == 0 || strcmp(argv[arg_index], "-h") == 0)
            {
                printf("Usage: ccalc [--interactive [--quiet]] [--help | --version | --commands [N]]\n"
                       "Each switch can be abbreviated by -i, -q etc.\n");
                return EXIT_SUCCESS;
            }

            // Check for version switch as first argument
            if (strcmp(argv[arg_index], "--version") == 0 || strcmp(argv[arg_index], "-v") == 0)
            {
                printf(COPYRIGHT_NOTICE);
                return EXIT_SUCCESS;
            }

            // Check for commands switch as first argument and parse subsequent arguments non-interactively
            if (strcmp(argv[arg_index], "--commands") == 0 || strcmp(argv[arg_index], "-c") == 0)
            {
                for (int i = arg_index + 1; i < argc; i++)
                {
                    if (!exec_command(argv[i]))
                    {
                        report_error("Error occurred in argument %d\n", i);
                        return EXIT_FAILURE;
                    }
                }

                if (!enter_interactive)
                {
                    return EXIT_SUCCESS;
                }
            }
            else
            {
                report_error("Unrecognized argument: %s\n", argv[arg_index]);
                return EXIT_FAILURE;
            }
        }
    }

    // If we are connected to a terminal, use readline and show whispered messages (interactive mode)
    set_interactive(isatty(STDIN_FILENO));
    if (!quiet)
    {
        whisper(COPYRIGHT_NOTICE
            "This program comes with ABSOLUTELY NO WARRANTY; for details type 'license'.\n"
            "This is free software, and you are welcome to redistribute it under certain conditions.\n");
    }
    // Enter loop to read all input lines, return appropiate exit code
    return process_input(stdin) ? EXIT_SUCCESS : EXIT_FAILURE;
}
