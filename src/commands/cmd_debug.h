#pragma once
#include "../parsing/context.h"

void cmd_debug_init();
bool cmd_debug_check(char *input);
void cmd_debug_exec(char *input);
