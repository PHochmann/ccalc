#pragma once
#include <stdbool.h>
#include <stdarg.h>

// Can be extended without further modifications
#define MAX_COLS 10

typedef enum
{
    BORDER_NONE,
    BORDER_SINGLE,
    BORDER_DOUBLE,
} BorderStyle;

typedef enum
{
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER,
    ALIGN_NUMBERS,
} TextAlignment;

struct Cell
{
    // User-supplied
    TextAlignment align;
    size_t span_x;
    size_t span_y;
    char *text;

    // Calculated
    bool is_set;
    bool text_needs_free;
    size_t x;
    size_t y;
    size_t padding;
    struct Cell *parent;
};

struct Row
{
    bool is_empty;
    BorderStyle hline_above;
    struct Cell cells[MAX_COLS];
    struct Row *next_row;
};

typedef struct
{
    // Number of columns (max. of num_cells over all rows)
    size_t num_cols;
    // Number of rows (length of linked list)
    size_t num_rows;
    // Vertical lines
    BorderStyle vlines[MAX_COLS + 1];
    // Start of linked list to rows
    struct Row *first_row;
    // Markers where to insert next cell
    struct Row *curr_row;
    size_t curr_col;
} Table;

// Settings
void set_decimal_separator(char *str);
char *get_decimal_separator();

// Data and printing
Table get_empty_table();
void print_table(Table *table);
void free_table(Table *table);

// Lines
void hline(Table *table, BorderStyle style);
void hline_at(Table *table, BorderStyle style, size_t num, ...);
void vline(Table *table, BorderStyle style);
void vline_at(Table *table, BorderStyle style, size_t num, ...);
void make_boxed(Table *table, BorderStyle style);

// Control
void set_position(Table *table, size_t x, size_t y);
void next_row(Table *table);
void set_alignment(Table *table, TextAlignment align);

// Cell insertion
void add_empty_cell(Table *table);
void add_cell(Table *table, TextAlignment align, char *text);
void add_cell_span(Table *table, TextAlignment align, size_t span_x, size_t span_y, char *text);
void add_cell_fmt(Table *table, TextAlignment align, char *fmt, ...);
void v_add_cell_fmt(Table *table, TextAlignment align, char *fmt, va_list args);
void add_cell_fmt_span(Table *table, TextAlignment align, size_t span_x, size_t span_y, char *fmt, ...);
void v_add_cell_fmt_span(Table *table, TextAlignment align, size_t span_x, size_t span_y, char *fmt, va_list args);
void add_cells_from_array(Table *table, size_t width, size_t height, char **array, TextAlignment *alignments);
