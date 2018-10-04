#include "arith/commands.h"

int main(int argc, char *argv[])
{
    init_commands();
    
    if (argc > 1)
    {
        make_silent();
        for (int i = 1; i < argc; i++) parse_input(argv[i]);
    }
    else
    {
        main_interactive();
    }
    
    return 0;
}
