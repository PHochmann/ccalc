#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "printing.h"
#include "constraint.h"
#include "text.h"

void add_cell_internal(Table *table, char *text, bool needs_free)
{
    if (table->curr_col >= MAX_COLS) return;

    struct Cell *cell = &table->curr_row->cells[table->curr_col];
    if (cell->is_set) return;

    cell->is_set = true;
    cell->text_needs_free = needs_free;
    cell->text = text;

    if (table->curr_col >= table->num_cols)
    {
        table->num_cols = table->curr_col + 1;
    }

    while (table->curr_col != MAX_COLS && table->curr_row->cells[table->curr_col].is_set)
    {
        table->curr_col++;
    }
}

struct Row *malloc_row(size_t y)
{
    struct Row *res = calloc(1, sizeof(struct Row));
    for (size_t i = 0; i < MAX_COLS; i++)
    {
        res->cells[i] = (struct Cell){
            .is_set = false,
            .x = i,
            .y = y,
            .parent = NULL,
            .text = NULL,
            .override_align = false,
            .override_border_left = false,
            .override_border_above = false,
            .span_x = 1,
            .span_y = 1,
            .text_needs_free = false,
            .dot_padding = 0
        };
    }
    return res;
}

struct Row *append_row(Table *table)
{
    struct Row *row = table->curr_row;
    while (row->next_row != NULL)
    {
        row = row->next_row;
    }
    row->next_row = malloc_row(table->num_rows);
    table->num_rows++;
    return row->next_row;
}

struct Row *get_row(Table *table, size_t index)
{
    struct Row *row = table->first_row;
    while (index-- > 0 && row != NULL) row = row->next_row;
    return row;
}

struct Cell *get_curr_cell(Table *table)
{
    return &table->curr_row->cells[table->curr_col];
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ User-functions ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/*
Returns: A new table with a single, empty row.
*/
Table get_empty_table()
{
    struct Row *first_row = malloc_row(0);
    return (Table){
        .num_cols = 0,
        .curr_col = 0,
        .first_row = first_row,
        .curr_row = first_row,
        .num_rows = 1,
        .alignments = { ALIGN_LEFT },
        .borders_left = { BORDER_NONE },
        .border_left_counters = { 0 }
    };
}

/*
Summary: Prints table to stdout
*/
void print_table(Table *table)
{
    print_table_internal(table);
}

void free_row(Table *table, struct Row *row)
{
    for (size_t i = 0; i < table->num_cols; i++)
    {
        if (row->cells[i].text_needs_free)
        {
            free(row->cells[i].text);
        }
    }
    free(row);
}

/*
Summary: Frees all rows and content strings in cells created by add_cell_fmt.
    Don't use the table any more, get a new one!
*/
void free_table(Table *table)
{
    struct Row *row = table->first_row;
    while (row != NULL)
    {
        struct Row *next_row = row->next_row;
        free_row(table, row);
        row = next_row;
    }
}

void set_position(Table *table, size_t x, size_t y)
{
    if (x >= MAX_COLS) return;
    table->curr_col = x;
    if (y < table->num_rows)
    {
        table->curr_row = get_row(table, y);
    }
    else
    {
        while (y >= table->num_rows)
        {
            table->curr_row = append_row(table);
        }
    }
}

/*
Summary: Next inserted cell will be inserted at first unset column of next row.
*/
void next_row(Table *table)
{
    if (table == NULL) return;

    // Extend linked list if necessary
    table->curr_col = 0;
    if (table->curr_row->next_row == NULL)
    {
        table->curr_row = append_row(table);
    }
    else
    {
        table->curr_row = table->curr_row->next_row;
        while (table->curr_col < MAX_COLS && table->curr_row->cells[table->curr_col].is_set)
        {
            table->curr_col++;
        }
    }
}

/*
Summary: Adds next cell. Buffer is not copied. print_table will access it.
    Ensure that lifetime of buffer outlasts last call of print_table!
*/
void add_cell(Table *table, char *text)
{
    add_cell_internal(table, text, false);
}

/*
Summary: Same as add_cell, but frees buffer on reset
*/
void add_cell_gc(Table *table, char *text)
{
    add_cell_internal(table, text, true);
}

void add_empty_cell(Table *table)
{
    add_cell_internal(table, NULL, false);
}

/*
Summary: Adds next cell and maintains buffer of text.
    Use add_cell to save memory if you maintain a content string yourself.
*/
void add_cell_fmt(Table *table, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    v_add_cell_fmt(table, fmt, args);
    va_end(args);
}

void v_add_cell_fmt(Table *table, char *fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    char *buffer = malloc(needed * sizeof(char));
    vsnprintf(buffer, needed, fmt, args_copy);
    add_cell_internal(table, buffer, true);
    va_end(args_copy);
}

/*
Summary: Puts contents of memory-contiguous 2D array into table cell by cell.
    Strings are not copied. Ensure that lifetime of array outlasts last call of print_table.
    Position of next insertion is first cell in next row.
*/
void add_cells_from_array(Table *table, size_t width, size_t height, char **array)
{
    if (table->curr_col + width > MAX_COLS) return;
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            add_cell(table, *(array + i * width + j));
        }
        next_row(table);
    }
}

void set_default_alignments(Table *table, size_t num_alignments, TextAlignment *alignments)
{
    for (size_t i = 0; i < num_alignments; i++)
    {
        table->alignments[i] = alignments[i];
    }
}

void override_alignment(Table *table, TextAlignment alignment)
{
    get_curr_cell(table)->align = alignment;
    get_curr_cell(table)->override_align = true;
}

void set_hline(Table *table, BorderStyle style)
{
    if (table->curr_row->border_above != BORDER_NONE)
    {
        table->curr_row->border_above_counter--;
    }
    if (style != BORDER_NONE)
    {
        table->curr_row->border_above_counter++;
    }
    table->curr_row->border_above = style;
}

void set_vline(Table *table, BorderStyle style)
{
    if (table->num_cols <= table->curr_col)
    {
        table->num_cols = table->curr_col + 1;
    }
    if (table->borders_left[table->curr_col] != BORDER_NONE)
    {
        table->border_left_counters[table->curr_col]--;
    }
    if (style != BORDER_NONE)
    {
        table->border_left_counters[table->curr_col]++;
    }

    table->borders_left[table->curr_col] = style;
}

void make_boxed(Table *table, BorderStyle style)
{
    set_position(table, 0, 0);
    set_vline(table, style);
    set_hline(table, style);
    set_position(table, table->num_cols, table->num_rows - 1);
    set_vline(table, style);
    set_hline(table, style);
}

void override_left_border(Table *table, BorderStyle style)
{
    if (get_curr_cell(table)->override_border_left && get_curr_cell(table)->border_left != BORDER_NONE)
    {
        table->border_left_counters[table->curr_col]--;
    }
    if (style != BORDER_NONE)
    {
        table->border_left_counters[table->curr_col]++;
    }

    get_curr_cell(table)->border_left = style;
    get_curr_cell(table)->override_border_left = true;
}

void override_above_border(Table *table, BorderStyle style)
{
    if (get_curr_cell(table)->override_border_above && get_curr_cell(table)->border_above != BORDER_NONE)
    {
        table->curr_row->border_above_counter--;
    }
    if( style != BORDER_NONE)
    {
        table->curr_row->border_above_counter++;
    }

    get_curr_cell(table)->border_above = style;
    get_curr_cell(table)->override_border_above = true;
}

/*
Summary: todo
*/
void set_span(Table *table, size_t span_x, size_t span_y)
{
    struct Cell *cell = &table->curr_row->cells[table->curr_col];
    struct Row *row = table->curr_row;
    size_t col = table->curr_col;

    cell->span_x = span_x;
    cell->span_y = span_y;

    // Inserts rows and sets child cells
    for (size_t i = 0; i < span_y; i++)
    {
        for (size_t j = 0; j < span_x; j++)
        {
            if (i == 0 && j == 0) continue;
            struct Cell *child = &table->curr_row->cells[cell->x + j];

            if (!child->is_set)
            {
                child->is_set = true;
                child->parent = cell;
                
                if (j != 0)
                {
                    child->border_left = BORDER_NONE;
                    child->override_border_left = true;
                }
                if (i != 0)
                {
                    child->border_above = BORDER_NONE;
                    child->override_border_above = true;
                }
            }
            else
            {
                // Span clashes with already set cell, truncate it and finalize method
                cell->span_y = i;
                cell->span_x = j;
                goto continuation;
            }
        }

        if (i + 1 < span_y) next_row(table);
    }
    continuation:
    table->curr_row = row;
    table->curr_col = col;
}
