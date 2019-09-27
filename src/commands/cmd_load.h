#pragma once
#include "../parsing/context.h"

void cmd_load_init();
bool cmd_load_check(char *input);
void cmd_load_exec(char *input);
