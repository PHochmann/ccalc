#include <stdbool.h>

#include "context.h"
#include "node.h"
#include "parser.h"

char* perr_to_string(ParserError perr);
bool begins_with(char *prefix, char *string);
bool indicate_abbreviation(char *string, size_t buffer_size);
void print_table(int num_rows, int num_cols, char **cells, bool head_border);
