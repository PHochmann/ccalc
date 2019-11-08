#include <stdbool.h>
#include "node.h"

#define OP_COLOR    "\x1B[47m\x1B[22;30m" // White background, black foreground
#define CONST_COLOR "\x1B[1;33m"          // Yellow
#define VAR_COLOR   "\x1B[1;36m"          // Cyan
#define COL_RESET   "\x1B[0m"

#define CONSTANT_TYPE_FMT "%-.10g"

size_t tree_to_string(Node *node, char *buffer, size_t buffer_size, bool color);
size_t sizeof_tree_to_string(Node *node, bool color);
void unsafe_tree_to_string(Node *node, char *buffer, bool color);
void print_tree(Node *node, bool color);
void print_tree_visually(Node *node);
