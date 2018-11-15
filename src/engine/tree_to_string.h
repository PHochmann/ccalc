#include "context.h"
#include "node.h"

void print_tree_visual(ParsingContext *ctx, Node *node);
size_t tree_inline(ParsingContext *ctx, Node *node, char *out_buffer, size_t buffer_size, bool colours);
