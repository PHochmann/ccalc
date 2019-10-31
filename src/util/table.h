#include <stdbool.h>

#define MAX_COLS 10
#define MAX_ROWS 100

typedef enum
{
    TEXTPOS_LEFT,
    TEXTPOS_RIGHT,
    TEXTPOS_CENTER
} TextPosition;

struct Cell
{
    TextPosition textpos;
    bool free_on_reset;
    char *text;
};

typedef struct
{
    // Table dimensions
    size_t num_cols;
    size_t num_rows;
    // Marker which cell is inserted next
    size_t x;
    size_t y;
    // Max. ocurring widths and heights in columns and rows
    int col_widths[MAX_COLS];
    int row_heights[MAX_ROWS];

    size_t num_hlines;
    size_t hlines[MAX_ROWS];
    struct Cell cells[MAX_COLS][MAX_ROWS];
} Table;

Table get_table();
void reset_table(Table *table);
void add_cell(Table *table, TextPosition textpos, char *buffer);
void add_cell_fmt(Table *table, TextPosition textpos, char *fmt, ...);
void add_cells_from_array(Table *table, size_t x, size_t y, size_t width, size_t height, char *array[height][width], ...);
void next_row(Table *table);
void hline(Table *table);
void set_position(Table *table, size_t col, size_t row);
void print_table(Table *table, bool borders);
