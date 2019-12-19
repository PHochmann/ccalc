#pragma once
#include <stdbool.h>
#include <stdio.h>

bool g_error;

void init_commands();
void unload_commands();
void process_input(FILE *file);
void parse_command(char *input);
