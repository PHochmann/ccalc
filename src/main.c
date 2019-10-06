#include <unistd.h>
#include "console_util.h"
#include "commands/core.h"

/*
Simple calculator written in C in which you can define your own constants and functions
https://github.com/PhilippHochmann/Calculator
(c) 2019 Philipp Hochmann, phil.hochmann [at] gmail [dot] com
*/
int main(int argc, char *argv[])
{
    // Build arithmetic context, initialise commands
    init_commands();
    
    // Parse any arguments non-interactively
    for (int i = 1; i < argc; i++)
    {
        parse_command(argv[i]);
    }
    
    // If we are connected to a shell, use readline and show whispered messages (interactive mode)
    if (isatty(STDIN_FILENO))
    {
        set_interactive(true);
    }

    // Enter loop to read all input lines
    process_input(stdin);
    
    return EXIT_SUCCESS;
}
