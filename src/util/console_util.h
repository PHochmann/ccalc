#pragma once
#include <stdbool.h>
#include <stdio.h>

void unload_console_util();
void init_console_util();
bool set_interactive(bool value);
bool is_interactive();
void whisper(const char *format, ...);
bool ask_input(FILE *file, char **out_input, const char *prompt_fmt, ...);
void report_error(const char *fmt, ...);
void software_defect(const char *fmt, ...);
void show_error_with_position(int pos, int length, const char *fmt, ...);
