#include "../engine/node.h"
#include "../engine/parser.h"

bool g_interactive; // When set to true, whispered prints will be displayed and readline will be used
bool g_debug; // When set to true, help will contain more information and AST of evaluation is shown

void init_util();
char *perr_to_string(ParserError perr);
void whisper(const char *format, ...);
bool ask_input(char *prompt, FILE *file, char **out_input);
bool parse_input_wrapper(ParsingContext *ctx, char *input, bool pad_parentheses, Node **out_res, bool apply_rules, bool apply_ans, bool constant);

void quit_init();
bool quit_check(char *input);
void quit_exec(ParsingContext *ctx, char *input);
void debug_init();
bool debug_check(char *input);
void debug_exec(ParsingContext *ctx, char *input);
