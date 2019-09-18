#pragma once
#include "../parsing/context.h"

void cmd_tree_init();
bool cmd_tree_check(char *input);
void cmd_tree_exec(ParsingContext *ctx, char *input);
