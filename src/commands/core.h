#pragma once
#include <stdio.h>

void init_commands();
void unload_commands();
void process_input(FILE *file);
void parse_command(char *input);
