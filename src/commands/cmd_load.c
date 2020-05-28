#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "cmd_load.h"
#include "commands.h"
#include "../console_util.h"
#include "../string_util.h"

#define COMMAND "load "

int cmd_load_check(char *input)
{
    return begins_with(COMMAND, input) ? 1 : 0;
}

/*
Summary: Opens file and processes its content as from stdin
*/
bool cmd_load_exec(char *input, __attribute__((unused)) int code)
{
    FILE *file = fopen(input + strlen(COMMAND), "r");

    if (file == NULL)
    {
        printf("Error loading file: %s.\n", strerror(errno));
        return false;
    }
    
    whisper("File successfully loaded.\n");
    // Set g_interactive to false to read with getline from file
    bool interactive = set_interactive(false);
    process_input(file);
    set_interactive(interactive);
    fclose(file);
    return true;
}
