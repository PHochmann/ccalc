#include <stdbool.h>

#define MAX_COLS 10
#define MAX_ROWS 100

typedef enum
{
    TEXTPOS_LEFT,
    TEXTPOS_RIGHT,
    TEXTPOS_CENTER
} TextPosition;

void reset_table();
void add_cell(TextPosition textpos, char *fmt, ...);
void add_cells_from_array(size_t x, size_t y, size_t width, size_t height, char *array[height][width], ...);
void next_row();
void hline();
void set_position(size_t col, size_t row);
void print_table(bool borders);
