#pragma once
#include <stdbool.h>
#include <stdarg.h>

// Can be extended without further modifications
#define MAX_COLS 10

typedef enum
{
    BORDER_NONE   = 0,
    BORDER_SINGLE = 1,
    BORDER_DOUBLE = 2,
} BorderStyle;

typedef enum
{
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER,
    ALIGN_NUMBERS, // Aligned at dot, rightmost, DOES NOT SUPPORT \n
} TextAlignment;

typedef struct
{
    TextAlignment align;
    size_t span_x;
    size_t span_y;
    BorderStyle border_left;
    BorderStyle border_above;
} CellSettings;

typedef struct Cell
{
    // User-supplied (or generated for child cells)
    CellSettings settings;
    char *text;

    // Generated
    bool is_set;
    bool text_needs_free;
    size_t x;
    size_t y;
    size_t dot_padding;
    struct Cell *parent;
} Cell;

struct Row
{
    Cell cells[MAX_COLS];
    struct Row *next_row;
    // Info whether a hline has been set
    bool hline;
};

typedef struct
{
    // Number of columns (max. of num_cells over all rows)
    size_t num_cols;
    // Number of rows (length of linked list)
    size_t num_rows;
    // Start of linked list to rows
    struct Row *first_row;
    // Markers where to insert next cell
    struct Row *curr_row;
    size_t curr_col;
    // Info whether a vline has been set
    bool vlines[MAX_COLS];
} Table;

// Data and printing
Table get_empty_table();
void print_table(Table *table);
void free_table(Table *table);

// Lines
void horizontal_line(Table *table, BorderStyle style, size_t num, ...);
void vertical_line(Table *table, BorderStyle style, size_t num, ...);
void make_boxed(Table *table, BorderStyle style);

// Control
void set_position(Table *table, size_t x, size_t y);
void next_row(Table *table);
void set_alignment_of_col(Table *table, TextAlignment align, size_t col, bool exclude_header);

// Cell insertion
void add_empty_cell(Table *table);
void add_standard_cell(Table *table, char *text);
void add_cell(Table *table, CellSettings settings, char *text);
void add_cell_at(Table *table, size_t x, size_t y, CellSettings settings, char *text);
void add_managed_cell(Table *table, CellSettings settings, char *text);
void add_standard_cell_fmt(Table *table, char *fmt, ...);
void add_cell_fmt(Table *table, CellSettings settings, char *fmt, ...);
void v_add_cell_fmt(Table *table, CellSettings settings, char *fmt, va_list args);
void add_from_array(Table *table, size_t width, size_t height, TextAlignment *col_aligns, char **array);

// Settings
CellSettings get_standard_settings();
CellSettings get_settings_align(TextAlignment align);
CellSettings get_settings_align_span(TextAlignment align, size_t span_x, size_t span_y);
CellSettings get_settings_align_span_border(TextAlignment align, size_t span_x, size_t span_y, BorderStyle border_left, BorderStyle border_above);
void change_settings(Table *table, CellSettings settings);
void change_settings_at(Table *table, size_t x, size_t y, CellSettings settings);
