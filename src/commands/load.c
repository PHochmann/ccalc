#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "load.h"
#include "core.h"
#include "console_util.h"
#include "../engine/string_util.h"

#define COMMAND "load "

void load_init() { }

bool load_check(char *input)
{
    return begins_with(COMMAND, input);
}

/*
Summary: Opens file and processes its content as from stdin
*/
void load_exec(__attribute__((unused)) ParsingContext *ctx, char *input)
{
    FILE *file = fopen(input + strlen(COMMAND), "r");

    if (file == NULL)
    {
        printf("Error loading file: %s\n", strerror(errno));
        return;
    }
    
    whisper("File successfully loaded\n");

    // Set g_interactive to false to read with getline from file
    bool interactive = set_interactive(false);
    process_input(file);
    set_interactive(interactive);
    
    fclose(file);
}
