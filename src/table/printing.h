#include "table.h"

TextAlignment get_align(TextAlignment default_align, struct Cell *cell);
void print_table_internal(Table *table);
