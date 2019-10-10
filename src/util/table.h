#include <stdbool.h>

#define MAX_COLS 10
#define MAX_ROWS 100

void reset_table();
void add_cell(char *fmt, ...);
void add_cell_buffer(char *buffer);
void next_row();
void set_position(size_t col, size_t row);
void print_table(bool head_border);
