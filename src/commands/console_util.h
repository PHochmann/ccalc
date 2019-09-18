#include <stdarg.h>
#include <stdio.h>

#include "../parsing/node.h"
#include "../parsing/parser.h"

bool g_interactive; // When set to true, whispered prints will be displayed and readline will be used
bool g_debug; // When set to true, help will contain more information and AST and inlined result is shown
Node *g_ans; // Constant node that contains result of last evaluation

void init_util();
char *perr_to_string(ParserError perr);
bool set_interactive(bool value);
void print_tree_inlined(ParsingContext *ctx, Node *node, bool color);
void whisper(const char *format, ...);
bool ask_input(char *prompt, FILE *file, char **out_input);
bool parse_input_wrapper_with_error(ParsingContext *ctx, char *input, char *error_fmt, Node **out_res, bool apply_rules, bool replace_ans, bool constant);
bool parse_input_wrapper(ParsingContext *ctx, char *input, Node **out_res, bool apply_rules, bool replace_ans, bool constant);
