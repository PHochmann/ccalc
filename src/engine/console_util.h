#include "context.h"
#include "node.h"
#include "parser.h"

char* perr_to_string(ParserError perr);
void print_op_info(ParsingContext *ctx, char *name);
void print_table(int num_rows, int num_cols, char** cells);
