#include <stdbool.h>

#include "node.h"
#include "../parsing/context.h"
#include "../../util/string_builder.h"

#define OP_COLOR    "\x1B[47m\x1B[22;30m" // White background, black foreground
#define CONST_COLOR "\x1B[1;33m"          // Yellow
#define VAR_COLOR   "\x1B[1;36m"          // Cyan
#define COL_RESET   "\x1B[0m"

#define CONSTANT_TYPE_FMT "%-.10g"

char *tree_to_str(const Node *node);
char *tree_to_str_fancy(const Node *node, const ParsingContext *ctx, bool color);
void tree_append_to_strbuilder(StringBuilder *builder, const Node *node, const ParsingContext *ctx, bool color);
void print_tree(const Node *node);
void print_tree_fancy(const Node *node, const ParsingContext *ctx, bool color);
void print_tree_visually(const Node *node);
