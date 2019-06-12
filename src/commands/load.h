#pragma once
#include "../engine/context.h"

void load_init();
bool load_check(char *input);
void load_exec(ParsingContext *ctx, char *input);
