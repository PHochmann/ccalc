#include "table.h"

TextAlignment get_align(TextAlignment default_align, const struct Cell *cell);
void fprint_table_internal(Table *table, FILE *stream);
