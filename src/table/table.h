#pragma once
#include <stdbool.h>
#include <stdarg.h>

// Can be changed without further modifications
#define MAX_COLS 11
// For ALIGN_NUMBERS
#define DECIMAL_SEPARATOR '.'

typedef enum
{
    BORDER_NONE,
    BORDER_SINGLE,
    BORDER_DOUBLE
} BorderStyle;

typedef enum
{
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER, // Centered (rounded to the left)
    ALIGN_NUMBERS // Aligned at dot, rightmost, does not support \n
} TextAlignment;

typedef struct Table Table;

// Data and printing
Table *get_empty_table();
void print_table(Table *table);
void fprint_table(Table *table, FILE *stream);
void free_table(Table *table);

// Control
void set_position(Table *table, size_t x, size_t y);
void next_row(Table *table);

// Cell insertion
void add_empty_cell(Table *table);
void add_cell(Table *table, const char *text);
void add_cell_gc(Table *table, char *text);
void add_cell_fmt(Table *table, const char *fmt, ...);
void add_cell_vfmt(Table *table, const char *fmt, va_list args);
void add_cells_from_array(Table *table, size_t width, size_t height, const char **array);

// Settings
void set_default_alignments(Table *table, size_t num_alignments, const TextAlignment *alignments);
void override_alignment(Table *table, TextAlignment alignment);
void override_alignment_of_row(Table *table, TextAlignment alignment);
void set_hline(Table *table, BorderStyle style);
void set_vline(Table *table, size_t index, BorderStyle style);
void make_boxed(Table *table, BorderStyle style);
void set_all_vlines(Table *table, BorderStyle style);
void override_left_border(Table *table, BorderStyle style);
void override_above_border(Table *table, BorderStyle style);
void set_span(Table *table, size_t span_x, size_t span_y);
