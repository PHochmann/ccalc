#pragma once
#include "../parsing/context.h"

void cmd_help_init();
bool cmd_help_check(char *input);
void cmd_help_exec(ParsingContext *ctx, char *input);
