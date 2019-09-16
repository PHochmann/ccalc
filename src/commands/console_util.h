#include <stdarg.h>
#include <stdio.h>
#include "../engine/node.h"
#include "../engine/parser.h"

bool g_interactive; // When set to true, whispered prints will be displayed and readline will be used
bool g_debug; // When set to true, help will contain more information and AST and inlined result is shown

void init_util();
char *perr_to_string(ParserError perr);
bool set_interactive(bool value);
void print_tree_inlined(ParsingContext *ctx, Node *node, bool color);
void whisper(const char *format, ...);
bool ask_input(char *prompt, FILE *file, char **out_input);
bool parse_input_wrapper(ParsingContext *ctx, char *input, Node **out_res, bool apply_rules, bool apply_ans, bool constant);
