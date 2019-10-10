#include <stdbool.h>
#include "../parsing/node.h"

#define OP_COLOR    "\x1B[47m\x1B[22;30m" // White background, black foreground
#define CONST_COLOR "\x1B[1;33m"          // Yellow
#define VAR_COLOR   "\x1B[1;36m"          // Cyan
#define COL_RESET   "\033[0m"
#define CONSTANT_TYPE_FMT "%-.10g"

bool begins_with(char *prefix, char *string);
void print_tree_visual(Node *node);
size_t tree_inline(Node *node, char *buffer, size_t buffer_size, bool color);
