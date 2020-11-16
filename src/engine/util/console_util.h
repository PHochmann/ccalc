#pragma once
#include <stdbool.h>
#include <stdio.h>

void unload_console_util();
void init_console_util();
bool set_show_errors(bool value);
bool set_interactive(bool value);
void whisper(const char *format, ...);
bool ask_input(FILE *file, char **out_input, const char *prompt_fmt, ...);
void report_error(const char *fmt, ...);
void software_defect(const char *fmt, ...);
