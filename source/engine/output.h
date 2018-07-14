#include "context.h"
#include "node.h"
#include "parser.h"

void show_tree(ParsingContext *ctx, Node *node);
char* perr_to_string(ParserError perr);
void print_ops(ParsingContext *ctx);