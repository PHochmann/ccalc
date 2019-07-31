#pragma once
#include <stdbool.h>
#include <stdarg.h>

#include "../engine/context.h"
#include "../engine/node.h"

#define VERSION "1.3.1"

void init_commands();
bool set_interactive(bool value);
void process_input(FILE *file);
void parse_command(char *input);
