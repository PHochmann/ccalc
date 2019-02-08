#pragma once
#include <stdbool.h>
#include <stdarg.h>

#include "../engine/node.h"

void init_commands();
void make_silent();
void main_interactive();
void parse_command(char *input);