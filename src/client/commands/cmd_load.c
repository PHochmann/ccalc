#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "../../util/console_util.h"
#include "../../util/string_util.h"
#include "../simplification/simplification.h"
#include "cmd_load.h"
#include "commands.h"

#define COMMAND "load "
#define LOAD_SIMPLIFICATION "load simplification "

int cmd_load_check(const char *input)
{
    return begins_with(COMMAND, input);
}

/*
Summary: Opens file and processes its content as from stdin
*/
bool cmd_load_exec(char *input, __attribute__((unused)) int code)
{
    if (begins_with(LOAD_SIMPLIFICATION, input))
    {
        input += strlen(LOAD_SIMPLIFICATION);
        unload_simplification();
        ssize_t count = init_simplification(input);
        if (count < 0)
        {
            report_error("File not found or readable\n");
            return false;
        }
        else
        {
            whisper("Successfully loaded simplification (%ld rules)\n", count);
            return true;
        }
    }
    else
    {
        // Normal load command
        FILE *file = fopen(input + strlen(COMMAND), "r");

        if (file == NULL)
        {
            report_error("Error loading file: %s\n", strerror(errno));
            return false;
        }
        
        // Set g_interactive to false to read with getline from file
        bool interactive = set_interactive(false);
        process_input(file);
        set_interactive(interactive);
        fclose(file);
        return true;
    }
}
