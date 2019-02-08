#include <unistd.h>

#include "commands/core.h"

int main(int argc, char *argv[])
{
    init_commands();
    
    if (argc > 1)
    {
        make_silent();
        
        for (int i = 1; i < argc; i++)
        {
            parse_command(argv[i]);
        }
    }
    else
    {
        // If we pipe in arguments, do so silently
        if (!isatty(STDIN_FILENO))
        {
            make_silent();
        }

        main_interactive();
    }
    
    return 0;
}
