#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "../util/trie.h"
#include "../util/console_util.h"
#include "commands/commands.h"
#include "version.h"

/*
Params
    str: Can be NULL
    sw:  Pointer to bool that will be set to true when switch is set
         Will be automatically set to false beforehand
*/
void add_switch(Trie *switches, const char *str, bool *sw)
{
    if (str != NULL) TRIE_ADD_ELEM(switches, str, bool*, sw);
    *sw = false;
}

/*
Params
    arg: Argument from command line that should be parsed
Returns: Pointer to bool that was supplied in add_switch that has been set to true
         NULL if arg does not correspond to any switch
*/
bool *parse_arg(Trie *switches, const char *arg)
{
    bool **sw = NULL;
    if (trie_contains(switches, arg, (void**)&sw))
    {
        **sw = true;
        return *sw;
    }
    else
    {
        return NULL;
    }
}

int main(int argc, char **argv)
{
    // Build trie full of strings that can be used as switches
    // Payload: bool* that will be set to true if switch is set
    Trie switches = trie_create(sizeof(bool*));
    bool force_interactive;
    bool quiet;
    bool help;
    bool version;
    bool commands;
    int commands_index = -1;
    add_switch(&switches, "--interactive", &force_interactive);
    add_switch(&switches, "-i", &force_interactive);
    add_switch(&switches, "--quiet", &quiet);
    add_switch(&switches, "-q", &quiet);
    add_switch(&switches, "--help", &help);
    add_switch(&switches, "-h", &help);
    add_switch(&switches, "--version", &version);
    add_switch(&switches, "-v", &version);
    add_switch(&switches, "--commands", &commands);
    add_switch(&switches, "-c", &commands);
    for (int i = 1; i < argc; i++)
    {
        bool *sw = parse_arg(&switches, argv[i]);
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
    trie_destroy(&switches);

    // Now do stuff based on the switches that have been set
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

    // Since we know that we parse commands or enter interactive mode, build arithmetic context and initialize commands
    init_commands();
    atexit(unload_commands);
    
    // Parse supplied commands non-interactively
    if (commands_index != -1)
    {
        for (int i = commands_index + 1; i < argc; i++)
        {
            if (!exec_command(argv[i]))
            {
                report_error("Error occurred in command no. %d\n", i - commands_index);
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
