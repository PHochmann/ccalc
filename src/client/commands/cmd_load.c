#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "../../engine/util/console_util.h"
#include "../../engine/util/string_util.h"
#include "cmd_load.h"
#include "commands.h"

#define COMMAND "load "

int cmd_load_check(const char *input)
{
    return begins_with(COMMAND, input);
}

/*
Summary: Opens file and processes its content as from stdin
*/
bool cmd_load_exec(char *input, __attribute__((unused)) int code)
{
    FILE *file = fopen(input + strlen(COMMAND), "r");

    if (file == NULL)
    {
        report_error("Error loading file: %s.\n", strerror(errno));
        return false;
    }
    
    // Set g_interactive to false to read with getline from file
    bool interactive = set_interactive(false);
    process_input(file);
    set_interactive(interactive);
    fclose(file);
    return true;
}
