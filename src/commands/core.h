#pragma once
#include <stdbool.h>
#include <stdarg.h>
#include "../parsing/context.h"
#include "../parsing/node.h"

#define VERSION "1.3.7"

void init_commands();
void process_input(FILE *file);
void parse_command(char *input);
