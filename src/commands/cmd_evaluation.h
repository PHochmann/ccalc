#pragma once
#include "../parsing/context.h"

void cmd_evaluation_init();
bool cmd_evaluation_check(char *input);
void cmd_evaluation_exec(char *input);
