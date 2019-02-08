#pragma once
#include "../engine/context.h"

typedef void (*CommandInitHandler)();
typedef bool (*CommandCheckHandler)(char *input);
typedef void (*CommandExecHandler)(ParsingContext *ctx, char *input);

typedef struct
{
    CommandInitHandler initHandler;
    CommandCheckHandler checkHandler;
    CommandExecHandler execHandler;
    char *description;
} Command;

Command get_command(CommandInitHandler initHandler, CommandCheckHandler checkHandler, CommandExecHandler execHandler, char *description);