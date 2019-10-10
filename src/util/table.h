#include <stdbool.h>

#define MAX_COLS 10
#define MAX_ROWS 100

void reset_table();
void add_cell(char *fmt, ...);
void add_cell_from_buffer(char *buffer);
void add_cells_from_array(size_t x, size_t y, size_t width, size_t height, char *array[height][width]);
void next_row();
void set_position(size_t col, size_t row);
void print_table(bool head_border);
