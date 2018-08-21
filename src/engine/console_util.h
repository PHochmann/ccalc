#include "context.h"
#include "node.h"
#include "parser.h"

char* opplace_to_string(Op_Placement place);
char* perr_to_string(ParserError perr);

bool begins_with(char *prefix, char *string);
void print_padded(char *string, int total_length);
void print_repeated(char* string, int amount);
void print_table(int num_rows, int num_cols, char** cells);
