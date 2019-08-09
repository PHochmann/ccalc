#include <stdio.h>
#include <unistd.h>
#include "commands/core.h"

int main(int argc, char *argv[])
{
    // Build arithmetic context, initialise commands, set interactive=false
    init_commands();
    
    // Parse any arguments non-interactively
    for (int i = 1; i < argc; i++)
    {
        parse_command(argv[i]);
    }
    
    // If we are connected to a shell, set interactive=true to use readline
    if (isatty(STDIN_FILENO))
    {
        set_interactive(true);
    }

    // Enter loop to read input lines
    process_input(stdin);
    
    return EXIT_SUCCESS;
}
