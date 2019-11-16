#include <stdbool.h>

#define MAX_COLS 5

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

struct Row
{
    size_t num_cells;
    struct Cell cells[MAX_COLS];
    bool hline_above;
    int height;
    struct Row *next_row;
};

typedef struct
{
    // Table dimensions
    size_t num_cols;
    // Start of linked list to rows
    struct Row *first_row;
    // Marker where to insert next cell
    struct Row *curr_row;
    // Max. ocurring widths and heights in columns and rows
    int col_widths[MAX_COLS];
} Table;

Table get_empty_table();
void free_table(Table *table);
void add_cell(Table *table, TextPosition textpos, char *buffer);
void add_cell_fmt(Table *table, TextPosition textpos, char *fmt, ...);
void add_cells_from_array(Table *table, size_t width, size_t height, char **array, TextPosition *alignments);
void next_row(Table *table);
void hline(Table *table);
void print_table(Table *table, bool borders);
