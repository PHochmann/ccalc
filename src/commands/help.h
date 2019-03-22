#pragma once
#include "../engine/context.h"

void help_init();
bool help_check(char *input);
void help_exec(ParsingContext *ctx, char *input);
