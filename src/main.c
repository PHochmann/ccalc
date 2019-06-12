#include <stdio.h>
#include <unistd.h>
#include "commands/core.h"

int main(int argc, char *argv[])
{
    init_commands();
    
    for (int i = 1; i < argc; i++)
    {
        parse_command(argv[i]);
    }
    
    if (isatty(STDIN_FILENO))
    {
        set_interactive(true);
    }

    process_input(stdin);
    
    return EXIT_SUCCESS;
}
