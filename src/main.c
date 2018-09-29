#include <stdlib.h>
#include <stdio.h>

#include "commands.h"
#include "engine/parser.h"

#define VERSION "1.1.0"

void _exit()
{
    uninit_parser();
}

int main(int argc, char *argv[])
{
    atexit(_exit);

    init_parser();
    init_commands();
    
    if (argc > 1)
    {
        make_silent();
        for (int i = 1; i < argc; i++) parse_input(argv[i]);
    }
    else
    {
#ifdef DEBUG
        printf("Calculator %s Debug build (c) 2018, Philipp Hochmann\n", VERSION);
#else
        printf("Calculator %s (c) 2018, Philipp Hochmann\n", VERSION);
#endif

        printf("Commands: debug, help, def_func <name> <arity>, def_rule <before> -> <after>\n");
        main_interactive();
    }
    
    return 0;
}
