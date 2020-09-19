#pragma once
#include <stdbool.h>
#include <stdio.h>

extern bool g_interactive; // When set to true, whispered prints will be displayed and readline will be used

void unload_console_util();
void init_console_util();
bool set_interactive(bool value);
void whisper(const char *format, ...);
bool ask_input(FILE *file, char **out_input, char *prompt_fmt, ...);
void report_error(char *fmt, ...);
