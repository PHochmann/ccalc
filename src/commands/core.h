#pragma once
#include <stdbool.h>
#include <stdarg.h>

#include "../engine/context.h"
#include "../engine/node.h"

void init_commands();
void make_interactive();
void process_input(FILE *file);
void parse_command(char *input);
