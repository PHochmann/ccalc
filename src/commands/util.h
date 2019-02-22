#include "../engine/node.h"

bool g_silent; // When set to true, whispered prints will not be displayed
bool g_debug; // When set to true, help will contain more information and AST of evaluation is shown

void init_util();
void whisper(const char *format, ...);
bool ask_input(char *prompt, char **out_input);
bool parse_input_wrapper(ParsingContext *ctx, char *input, bool pad_parentheses, Node **out_res, bool apply_rules, bool apply_ans, bool constant);

void quit_init();
bool quit_check(char *input);
void quit_exec(ParsingContext *ctx, char *input);
void debug_init();
bool debug_check(char *input);
void debug_exec(ParsingContext *ctx, char *input);