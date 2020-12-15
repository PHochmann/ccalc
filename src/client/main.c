#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "version.h"
#include "argparse.h"
#include "util/console_util.h"
#include "commands/commands.h"

int main(int argc, char **argv)
{
    init_argparse();
    bool force_interactive;
    bool quiet;
    bool help;
    bool version;
    bool commands;
    int commands_index = -1;
    add_switch("--interactive", "-i", &force_interactive);
    add_switch("--quiet", "-q", &quiet);
    add_switch("--help", "-h", &help);
    add_switch("--version", "-v", &version);
    add_switch("--commands", "-c", &commands);
    for (int i = 1; i < argc; i++)
    {
        bool *sw = parse_arg(argv[i]);
        if (sw == &commands)
        {
            commands_index = i;
            break;
        }
        if (sw == NULL)
        {
            report_error("Unrecognized argument: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }
    unload_argparse();

    if (help)
    {
        printf("Usage: ccalc [--help] [--version] [--interactive] [--quiet] [--commands [<commands>]]\n"
               "Each switch can be abbreviated by -h, -v etc.\n");
        return EXIT_SUCCESS;
    }
    if (version)
    {
        printf(COPYRIGHT_NOTICE);
        return EXIT_SUCCESS;
    }

    // Print copyright notice if we are connected to a terminal and actually will enter interactive mode
    if (force_interactive || commands_index == -1)
    {
        if (!quiet && isatty(STDIN_FILENO))
        {
            printf(COPYRIGHT_NOTICE
                "This program comes with ABSOLUTELY NO WARRANTY; for details type 'license'.\n"
                "This is free software, and you are welcome to redistribute it under certain conditions.\n");
        }
    }

    // Build arithmetic context, initialize commands
    init_commands();
    // Free all resources at exit
    atexit(unload_commands);
    
    if (commands_index != -1)
    {
        for (int i = commands_index + 1; i < argc; i++)
        {
            if (!exec_command(argv[i]))
            {
                report_error("Error occurred in argument %d\n", i);
                return EXIT_FAILURE;
            }
        }
        if (!force_interactive)
        {
            return EXIT_SUCCESS;
        }
    }
    
    // If we are connected to a terminal, use readline and show whispered messages (interactive mode)
    set_interactive(isatty(STDIN_FILENO));
    // Enter loop to read all input lines, return appropiate exit code
    return process_input(stdin) ? EXIT_SUCCESS : EXIT_FAILURE;
}
