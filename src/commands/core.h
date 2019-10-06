#pragma once
#include <stdio.h>

#define VERSION "1.3.8"

void init_commands();
void process_input(FILE *file);
void parse_command(char *input);
