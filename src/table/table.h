#pragma once
#include <stdbool.h>
#include <stdarg.h>

// Can be changed without further modifications
#define MAX_COLS 10
// For ALIGN_NUMBERS
#define DECIMAL_SEPARATOR "."

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
    ALIGN_CENTER,  // Centered (rounded to the left)
    ALIGN_NUMBERS, // Aligned at dot, rightmost, DOES NOT SUPPORT \n YET
} TextAlignment;

struct Cell
{
    char *text; // Actual content to be displayed

    // Settings
    TextAlignment align;        // Non default, how to place text in col width
    BorderStyle border_left;    // Non-default border left
    BorderStyle border_above;   // Non-default border above
    size_t span_x;              // How many cols to span over
    size_t span_y;              // How many rows to span over
    bool override_align;        // Default set for each col in table
    bool override_border_left;  // Default set for each col in table
    bool override_border_above; // Default set in row

    // Generated
    bool is_set;          // Indicates whether data is valid
    bool text_needs_free; // When set to true, text will be freed on free_table
    size_t x;             // Column position
    size_t y;             // Row position
    size_t dot_padding;   // Right padding needed to align dots
    struct Cell *parent;  // Cell that spans into this cell
};

struct Row
{
    struct Cell cells[MAX_COLS]; // All cells of this row from left to right
    struct Row *next_row;        // Pointer to next row or NULL if last row
    BorderStyle border_above;    // Default border above (can be overwritten in cell)
    size_t border_above_counter; // Counts cells that override their border_above
};

typedef struct
{
    size_t num_cols;                       // Number of columns (max. of num_cells over all rows)
    size_t num_rows;                       // Number of rows (length of linked list)
    struct Row *first_row;                 // Start of linked list to rows
    struct Row *curr_row;                  // Marker of row of next inserted cell
    size_t curr_col;                       // Marker of col of next inserted cell
    BorderStyle borders_left[MAX_COLS];    // Default left border of cols
    TextAlignment alignments[MAX_COLS];    // Default alignment of cols
    size_t border_left_counters[MAX_COLS]; // Counts cells that override their border_left
} Table;

// Data and printing
Table get_empty_table();
void print_table(Table *table);
void free_table(Table *table);

// Control
void set_position(Table *table, size_t x, size_t y);
void next_row(Table *table);

// Cell insertion
void add_empty_cell(Table *table);
void add_cell(Table *table, char *text);
void add_cell_gc(Table *table, char *text);
void add_cell_fmt(Table *table, char *fmt, ...);
void add_cell_vfmt(Table *table, char *fmt, va_list args);
void add_cells_from_array(Table *table, size_t width, size_t height, char **array);

// Settings
void set_default_alignments(Table *table, size_t num_alignments, TextAlignment *alignments);
void override_alignment(Table *table, TextAlignment alignment);
void override_alignment_of_row(Table *table, TextAlignment alignment);
void set_hline(Table *table, BorderStyle style);
void set_vline(Table *table, BorderStyle style);
void make_boxed(Table *table, BorderStyle style);
void override_left_border(Table *table, BorderStyle style);
void override_above_border(Table *table, BorderStyle style);
void set_span(Table *table, size_t span_x, size_t span_y);
