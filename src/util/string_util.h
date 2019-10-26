#include <stdbool.h>

#include "../parsing/node.h"
#include "../parsing/parser.h"

#define OP_COLOR    "\x1B[47m\x1B[22;30m" // White background, black foreground
#define CONST_COLOR "\x1B[1;33m"          // Yellow
#define VAR_COLOR   "\x1B[1;36m"          // Cyan
#define COL_RESET   "\x1B[0m"

#define CONSTANT_TYPE_FMT "%-.10g"

size_t ansi_strlen(char *string);
bool begins_with(char *prefix, char *string);
char *perr_to_string(ParserError perr);
void print_tree_visually(Node *node);
size_t tree_to_string(Node *node, char *buffer, size_t buffer_size, bool color);
