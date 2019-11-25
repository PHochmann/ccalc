#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "constraint.h"
#include "text.h"

#define HLINE_SINGLE "─"
#define HLINE_DOUBLE "═"
#define VLINE_SINGLE "│"
#define VLINE_DOUBLE "║"
static const char *BORDERS_SINGLE[3][3] = {
    { "┌", "┬", "┐" },
    { "├", "┼", "┤" },
    { "└", "┴", "┘" },
};
static const char *BORDERS_DOUBLE[3][3] = {
    { "╔", "╦", "╗" },
    { "╠", "╬", "╣" },
    { "╚", "╩", "╝" },
};

void add_cell_internal(Table *table, TextAlignment align, size_t span_x, size_t span_y, char *text, bool needs_free)
{
    if (table->curr_col + span_x > MAX_COLS) return;

    struct Cell *cell = &table->curr_row->cells[table->curr_col];
    *cell = (struct Cell){
        .align = align,
        .span_x = span_x,
        .span_y = span_y,
        .text = text,
        .x = cell->x,
        .y = cell->y,
        .is_set = true,
        .text_needs_free = needs_free,
        .parent = NULL
    };

    // Insert rows and set child cells for span_x and span_y
    struct Row *curr_row = table->curr_row;
    size_t curr_col = table->curr_col;
    for (size_t i = 0; i < span_y; i++)
    {
        table->curr_row->is_empty = false;

        for (size_t j = 0; j < span_x; j++)
        {
            if (i == 0 && j == 0) continue;

            if (!table->curr_row->cells[cell->x + j].is_set)
            {
                table->curr_row->cells[cell->x + j].span_x = span_x;
                table->curr_row->cells[cell->x + j].is_set = true;
                table->curr_row->cells[cell->x + j].parent = cell;
            }
            else
            {
                // Span clashes with already set cell, truncate it and finalize method
                cell->span_y = i + 1;
                cell->span_x = j + 1;
                goto continuation;
            }
        }

        next_row(table);
    }
    continuation:
    table->curr_row = curr_row;
    table->curr_col = curr_col;

    // Update book-keeping info and next insertion marker
    if (table->curr_col + span_x > table->num_cols)
    {
        table->num_cols = table->curr_col + span_x;
    }

    while (table->curr_col != MAX_COLS && table->curr_row->cells[table->curr_col].is_set)
    {
        table->curr_col++;
    }
}

size_t get_total_width(Table *table, size_t *col_widths, size_t col, size_t span_x)
{
    size_t sum = 0;
    for (size_t i = 0; i < span_x; i++)
    {
        if (i < span_x - 1 && table->vlines[col + i + 1] != BORDER_NONE) sum++;
        sum += col_widths[col + i];
    }
    return sum;
}

void print_vline(BorderStyle style)
{
    switch (style)
    {
        case BORDER_SINGLE:
            printf(VLINE_SINGLE);
            break;
        case BORDER_DOUBLE:
            printf(VLINE_DOUBLE);
            break;
        case BORDER_NONE:
            break;
    }
}

void print_hline(BorderStyle style)
{
    switch (style)
    {
        case BORDER_SINGLE:
            printf(HLINE_SINGLE);
            break;
        case BORDER_DOUBLE:
            printf(HLINE_DOUBLE);
            break;
        case BORDER_NONE:
            break;
    }
}

void print_border_char(bool left, bool right, bool above, bool below, BorderStyle style)
{
    if (above && below && !left && !right)
    {
        print_vline(style);
        return;
    }
    if (!above && !below && left && right)
    {
        print_hline(style);
        return;
    }

    size_t matrix_y = 0;
    size_t matrix_x = 0;
    if (!above) matrix_y = 0;
    if (above && below) matrix_y = 1;
    if (above && !below) matrix_y = 2;
    if (!left) matrix_x = 0;
    if (left && right) matrix_x = 1;
    if  (left && !right) matrix_x = 2;

    if (style == BORDER_DOUBLE)
    {
        printf("%s", BORDERS_DOUBLE[matrix_y][matrix_x]);
    }
    else
    {
        if (style == BORDER_SINGLE)
        {
            printf("%s", BORDERS_SINGLE[matrix_y][matrix_x]);
        }
    }
}

size_t get_line_of_cell(struct Cell *cell, size_t line_index, char **out_start)
{
    if (cell->parent == NULL) return get_line_of_string(cell->text, line_index, out_start);
    return get_line_of_string(cell->parent->text, line_index, out_start);
}

bool has_left_border(struct Cell *cell)
{
    return cell->parent == NULL || cell->parent->x == cell->x;
}

/*bool has_right_border(struct Cell *cell)
{
    return cell->span_x == 1 && (cell->parent == NULL || cell->parent->span_x - 1 == cell->x - cell->parent->x);
}*/

bool has_upper_border(struct Cell *cell)
{
    return cell->parent == NULL || cell->parent->y == cell->y;
}

/*bool has_lower_border(struct Cell *cell)
{
    return cell->span_y == 1 && (cell->parent == NULL || cell->parent->span_y - 1 == cell->y - cell->parent->y)
}*/

// Above row can be NULL, below row must not be NULL!
void print_complete_line(Table *table,
    struct Row *above_row,
    struct Row *below_row,
    size_t *line_indices,
    size_t *col_widths)
{
    for (size_t i = 0; i < table->num_cols; i++)
    {
        // Print vline-hline intersection
        if (table->vlines[i] != BORDER_NONE)
        {
            bool above = above_row != NULL && has_left_border(&above_row->cells[i]);
            bool below = !below_row->is_empty && has_left_border(&below_row->cells[i]);
            bool left = i > 0 && has_upper_border(&below_row->cells[i - 1]);
            bool right = i < MAX_COLS - 1 && has_upper_border(&below_row->cells[i]);
            print_border_char(left, right, above, below, below_row->hline_above);
        }

        // Print -- in between intersections (or content when cell has span_y > 1)
        if (has_upper_border(&below_row->cells[i]))
        {
            switch (below_row->hline_above)
            {
                case BORDER_SINGLE:
                    print_repeated(HLINE_SINGLE, col_widths[i]);
                    break;
                case BORDER_DOUBLE:
                    print_repeated(HLINE_DOUBLE, col_widths[i]);
                    break;
                case BORDER_NONE:
                    break;
            }
        }
        else
        {
            struct Cell *parent = below_row->cells[i].parent;

            char *str = NULL;
            size_t len = get_line_of_cell(parent, line_indices[i], &str);
            line_indices[i]++;
            print_padded(str,
                len,
                get_total_width(table, col_widths, i, parent->span_x),
                parent->align);
            i += parent->span_x - 1;
        }
    }

    // Print last vline-hline intersection on right border
    if (table->vlines[table->num_cols] != BORDER_NONE)
    {
        bool above = above_row != NULL;
        bool below = !below_row->is_empty;
        bool left  = has_upper_border(&below_row->cells[table->num_cols - 1]);
        bool right = false;
        print_border_char(left, right, above, below, below_row->hline_above);
    }
    printf("\n");
}

/*
Summary: Prints table to stdout, optionally with border-box around it
*/
void print_table(Table *table)
{
    size_t col_widths[table->num_cols];
    size_t row_heights[table->num_rows];
    get_dimensions(table, col_widths, row_heights);

    printf("Dimensions: ");
    for (size_t i = 0; i < table->num_cols; i++) printf("%zu ", col_widths[i]);
    printf("; ");
    for (size_t i = 0; i < table->num_rows; i++) printf("%zu ", row_heights[i]);
    printf("\n");

    size_t line_indices[table->num_cols];
    for (size_t i = 0; i < table->num_cols; i++) line_indices[i] = 0;

    // Print rows
    struct Row *prev_row = NULL;
    struct Row *curr_row = table->first_row;
    size_t row_index = 0;
    while (curr_row != NULL)
    {
        if (curr_row->hline_above != BORDER_NONE)
        {
            print_complete_line(table,
                prev_row,
                curr_row,
                line_indices,
                col_widths);
        }

        if (curr_row->is_empty) break;

        // Reset line indices for newly beginning cell, don't reset them for cells that are children spanning from above
        for (size_t j = 0; j < table->num_cols; j++)
        {
            if (curr_row->cells[j].parent == NULL || curr_row->cells[j].y == curr_row->cells[j].parent->y)
            {
                line_indices[j] = 0;
            }
        }

        for (size_t j = 0; j < row_heights[row_index]; j++)
        {
            // Print cell
            for (size_t k = 0; k < table->num_cols; k += curr_row->cells[k].span_x)
            {
                print_vline(table->vlines[k]);

                char *str;
                size_t str_len;
                if (curr_row->cells[k].is_set)
                {
                    str = NULL;
                    str_len = get_line_of_cell(&curr_row->cells[k], line_indices[k], &str);
                }
                else
                {
                    str = "\0";
                    str_len = 0;
                }
                
                print_padded(str,
                    str_len,
                    get_total_width(table, col_widths, k, curr_row->cells[k].span_x),
                    curr_row->cells[k].align);
                
                line_indices[k]++;
            }
            print_vline(table->vlines[table->num_cols]);
            printf("\n");
        }

        row_index++;
        prev_row = curr_row;
        curr_row = curr_row->next_row;
    }
}

struct Row *malloc_row(size_t y)
{
    struct Row *res = calloc(1, sizeof(struct Row));
    *res = (struct Row){
        .hline_above = BORDER_NONE,
        .is_empty = true
    };
    for (size_t i = 0; i < MAX_COLS; i++)
    {
        res->cells[i] = (struct Cell){
            .is_set = false,
            .x = i,
            .y = y,
            .parent = NULL,
            .text = NULL,
            .span_x = 1,
            .span_y = 1,
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
    row->is_empty = false;
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

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ User-functions ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/*
Summary: Next inserted cell will be inserted at first unset column of next row.
*/
void next_row(Table *table)
{
    if (table == NULL) return;

    // Extend linked list if necessary
    table->curr_col = 0;
    table->curr_row->is_empty = false;
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
        .vlines = { BORDER_NONE }
    };
}

/*
Summary: Frees all rows and content strings in cells created by add_cell_fmt.
    Don't use the table any more, get a new one!
*/
void free_table(Table *table)
{
    struct Row *curr_row = table->first_row;
    while (curr_row != NULL)
    {
        for (size_t i = 0; i < table->num_cols; i++)
        {
            if (curr_row->cells[i].text_needs_free)
            {
                free(curr_row->cells[i].text);
            }
        }

        struct Row *next_row = curr_row->next_row;
        free(curr_row);
        curr_row = next_row;
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

void add_empty_cell(Table *table)
{
    add_cell(table, ALIGN_LEFT, "");
}

/*
Summary: Adds next cell. Buffer is not copied. print_table will access it.
    Ensure that lifetime of buffer outlasts last call of print_table!
*/
void add_cell_span(Table *table, TextAlignment align, size_t span_x, size_t span_y, char *text)
{
    add_cell_internal(table, align, span_x, span_y, text, false);
}

void v_add_cell_fmt_span(Table *table,
    TextAlignment align,
    size_t span_x,
    size_t span_y,
    char *fmt,
    va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    char *buffer = malloc(needed * sizeof(char));
    vsnprintf(buffer, needed, fmt, args_copy);
    add_cell_internal(table, align, span_x, span_y, buffer, true);
}

void add_cell(Table *table, TextAlignment align, char *text)
{
    add_cell_span(table, align, 1, 1, text);
}

/*
Summary: Adds next cell and maintains buffer of text.
    Use add_cell to save memory if you maintain a content string yourself.
*/
void add_cell_fmt(Table *table, TextAlignment align, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    v_add_cell_fmt_span(table, align, 1, 1, fmt, args);
    va_end(args);
}

void v_add_cell_fmt(Table *table, TextAlignment align, char *fmt, va_list args)
{
    v_add_cell_fmt_span(table, align, 1, 1, fmt, args);
}

void add_cell_fmt_span(Table *table, TextAlignment align, size_t span_x, size_t span_y, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    v_add_cell_fmt_span(table, align, span_x, span_y, fmt, args);
    va_end(args);
}

/*
Summary: Puts contents of memory-contiguous 2D array into table cell by cell.
    Strings are not copied. Ensure that lifetime of array outlasts last call of print_table.
    Position of next insertion is first cell in next row.
*/
void add_cells_from_array(Table *table, size_t width, size_t height, char **array, TextAlignment *alignments)
{
    if (table->curr_col + width > MAX_COLS) return;

    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            add_cell(table, alignments[j], *(array + i * width + j));
        }
        next_row(table);
    }
}

/*
Summary: Inserts horizontal line above current row.
*/
void hline(Table *table, BorderStyle style)
{
    table->curr_row->hline_above = style;
}

/*
Summary: Inserts vertical line right to current column.
*/
void vline(Table *table, BorderStyle style)
{
    table->vlines[table->curr_col] = style;
}

/*
Summary: Puts enclosing borders around current content of table
*/
void make_boxed(Table *table, BorderStyle style)
{
    table->vlines[0] = style;
    table->vlines[table->num_cols] = style;
    table->first_row->hline_above = style;
    struct Row *row = table->first_row;
    while (row->next_row != NULL) row = row->next_row;
    if (row->is_empty)
    {
        row->hline_above = style;
    }
    else
    {
        append_row(table)->hline_above = style;
    }
}

void add_full_lines(Table *table, BorderStyle style)
{
    for (size_t i = 0; i <= table->num_cols; i++)
    {
        table->vlines[i] = style;
    }
    struct Row *row = table->first_row;
    while (row != NULL)
    {
        row->hline_above = style;
        row = row->next_row;
    }
}