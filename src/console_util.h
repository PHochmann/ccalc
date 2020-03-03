#pragma once
#include <stdbool.h>
#include <stdio.h>

bool g_interactive; // When set to true, whispered prints will be displayed and readline will be used

void console_util_reset();
void console_util_init();
bool set_interactive(bool value);
void whisper(const char *format, ...);
bool ask_input(FILE *file, char **out_input, char *prompt_fmt, ...);
bool ask_yes_no(bool default_val);
