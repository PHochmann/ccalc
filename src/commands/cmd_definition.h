#pragma once
#include "../parsing/context.h"

void cmd_definition_init();
bool cmd_definition_check(char *input);
void cmd_definition_exec(char *input);
