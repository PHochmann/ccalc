#include <stdarg.h>
#include <stdio.h>

#include "../parsing/node.h"
#include "../parsing/parser.h"

bool g_interactive; // When set to true, whispered prints will be displayed and readline will be used

void unload_console_util();
void init_console_util();
bool set_interactive(bool value);
void print_tree(Node *node, bool color);
void whisper(const char *format, ...);
bool ask_input(FILE *file, char **out_input, char *prompt_fmt, ...);
bool parse_input_from_console(char *input,
    char *error_fmt,
    Node **out_res);
size_t split(char *str, char **out_strs, size_t num_delimiters, ...);
