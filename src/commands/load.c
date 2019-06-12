#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "load.h"
#include "core.h"
#include "util.h"
#include "../engine/constants.h"
#include "../engine/operator.h"
#include "../engine/string_util.h"

#define PREFIX "load "

void load_init()
{

}

bool load_check(char *input)
{
    return begins_with(PREFIX, input);
}

void load_exec(__attribute__((unused)) ParsingContext *ctx, char *input)
{
    FILE *file = fopen(input + strlen(PREFIX), "r");

    if (file == NULL)
    {
        printf("Error loading file: %s\n", strerror(errno));
        return;
    }

    bool interactive = set_interactive(false);
    process_input(file);
    set_interactive(interactive);
    
    fclose(file);
}
