#pragma once
#include "../engine/context.h"

void show_rules_init();
bool show_rules_check(char *input);
void show_rules_exec(ParsingContext *ctx, char *input);
