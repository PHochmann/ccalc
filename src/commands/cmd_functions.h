#pragma once
#include "../parsing/context.h"

void cmd_functions_init();
bool cmd_functions_check(char *input);
void cmd_functions_exec(ParsingContext *ctx, char *input);
