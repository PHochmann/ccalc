#pragma once
#include <stdbool.h>

#define VERSION "1.5.6"

#ifdef DEBUG
    #define RELEASE VERSION " DEBUG"
#else
    #define RELEASE VERSION
#endif

#define COPYRIGHT_NOTICE "ccalc " RELEASE " Copyright (C) 2020 Philipp Hochmann, phil.hochmann@gmail.com\n"

int cmd_help_check(const char *input);
bool cmd_help_exec(char *input, int code);
