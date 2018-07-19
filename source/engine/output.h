#include <stdarg.h>

#include "context.h"
#include "node.h"
#include "parser.h"

void show_tree(ParsingContext *ctx, Node *node);
char* perr_to_string(ParserError perr);
void print_ops(ParsingContext *ctx);
void print_op_info(ParsingContext *ctx, char *name);
void print_table(int num_rows, int num_cols, char** cells);