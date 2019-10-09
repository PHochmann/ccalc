#include <stdbool.h>
#include "parsing/node.h"

#define CONSTANT_TYPE_FMT "%-.10g"

bool begins_with(char *prefix, char *string);
void print_tree_visual(Node *node);
size_t tree_inline(Node *node, char *buffer, size_t buffer_size, bool color);
