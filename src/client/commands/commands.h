#pragma once
#include <stdbool.h>
#include <stdio.h>

void init_commands();
void unload_commands();
bool process_input(FILE *file);
bool exec_command(char *input);
