#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "constraint.h"
#include "text.h"

static char *BORDER_MATRIX_SINGLE[] = {
    "┌", "┬", "┐",
    "├", "┼", "┤",
    "└", "┴", "┘",
    "─", "│", " ",
};

static char *BORDER_MATRIX_DOUBLE[] = {
    "╔", "╦", "╗",
    "╠", "╬", "╣",
    "╚", "╩", "╝",
    "═", "║", " ",
};

static size_t HLINE_INDEX = 9;
static size_t VLINE_INDEX = 10;

// Index encodes whether a border intersects (0: no intersection, 1: intersection), clockwise
static size_t BORDER_LOOKUP[16] = { 11, 11, 11, 6, 11, 10, 0, 3, 11, 8, 9, 7, 2, 5, 1, 4 };

void add_cell_internal(Table *table, CellSettings settings, char *text, bool needs_free)
{
    if (table->curr_col + settings.span_x > MAX_COLS) return;

    Cell *cell = &table->curr_row->cells[table->curr_col];
    if (cell->is_set) return;

    *cell = (Cell){
        .settings = settings,
        .text = text,
        .x = cell->x,
        .y = cell->y,
        .is_set = true,
        .text_needs_free = needs_free,
        .parent = NULL,
        .dot_padding = 0
    };

    // Insert rows and set child cells for span_x and span_y
    struct Row *curr_row = table->curr_row;
    size_t curr_col = table->curr_col;
    CellSettings *parent_settings = &cell->settings;

    for (size_t i = 0; i < settings.span_y; i++)
    {
        for (size_t j = 0; j < settings.span_x; j++)
        {
            if (i == 0 && j == 0) continue;
            Cell *child = &table->curr_row->cells[cell->x + j];

            if (!child->is_set)
            {
                //child->settings = settings;
                child->is_set = true;
                child->parent = cell;
                //printf("%s", cell->text);
            }
            else
            {
                // Span clashes with already set cell, truncate it and finalize method
                parent_settings->span_y = i;
                parent_settings->span_x = j;
                goto continuation;
            }
        }

        if (i + 1 < settings.span_y) next_row(table);
    }
    continuation:
    table->curr_row = curr_row;
    table->curr_col = curr_col;

    // Update book-keeping info and next insertion marker
    if (settings.border_left != BORDER_NONE)
    {
        table->vlines[table->curr_col] = true;
    }

    if (settings.border_above != BORDER_NONE)
    {
        table->curr_row->hline = true;
    }

    if (table->curr_col + settings.span_x > table->num_cols)
    {
        table->num_cols = table->curr_col + settings.span_x;
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
        if (i < span_x - 1 && table->vlines[col + i + 1]) sum++;
        sum += col_widths[col + i];
    }
    return sum;
}

bool spans_from_above(Cell *cell)
{
    return cell->parent != NULL && cell->parent->y < cell->y;
}

bool spans_from_left(Cell *cell)
{
    return cell->parent != NULL && cell->parent->x < cell->x;
}

void print_intersection_char(Cell *right_above, Cell *left_below, Cell *right_below)
{
    BorderStyle above = right_above == NULL
        || spans_from_left(right_above) ? BORDER_NONE : right_above->settings.border_left;
    BorderStyle right = right_below == NULL
        || spans_from_above(right_below) ? BORDER_NONE : right_below->settings.border_above;
    BorderStyle below = right_below == NULL
        || spans_from_left(right_below) ? BORDER_NONE : right_below->settings.border_left;
    BorderStyle left  = left_below == NULL
        || spans_from_above(left_below) ? BORDER_NONE : left_below->settings.border_above;

    size_t index = 0;
    if (above != BORDER_NONE)
    {
        index |= (1 << 0);
    }
    if (right != BORDER_NONE)
    {
        index |= (1 << 1);
    }
    if (below != BORDER_NONE)
    {
        index |= (1 << 2);
    }
    if (left != BORDER_NONE)
    {
        index |= (1 << 3);
    }

    //printf("%zu ", index);
    //if (right_above != NULL) printf("%p", (void*)right_above->parent);

    int style_selection = above + right + below + left;
    if (style_selection > 4)
    {
        printf("%s", BORDER_MATRIX_DOUBLE[BORDER_LOOKUP[index]]);
    }
    else
    {
        printf("%s", BORDER_MATRIX_SINGLE[BORDER_LOOKUP[index]]);
    }
}

size_t get_line_of_cell(Cell *cell, size_t line_index, char **out_start)
{
    if (cell->parent == NULL) return get_line_of_string(cell->text, line_index, out_start);
    return get_line_of_string(cell->parent->text, line_index, out_start);
}

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
        if (table->vlines[i])
        {
            print_intersection_char(
                above_row != NULL ? &above_row->cells[i] : NULL,
                i > 0 ? &below_row->cells[i - 1] : NULL,
                &below_row->cells[i]);
        }

        // Print -- in between intersections (or content when cell has span_y > 1)
        if (below_row->cells[i].parent == NULL || below_row->cells[i].parent->y == below_row->cells[i].y)
        {
            switch (below_row->cells[i].settings.border_above)
            {
                case BORDER_SINGLE:
                    print_repeated(BORDER_MATRIX_SINGLE[HLINE_INDEX], col_widths[i]);
                    break;
                case BORDER_DOUBLE:
                    print_repeated(BORDER_MATRIX_DOUBLE[HLINE_INDEX], col_widths[i]);
                    break;
                case BORDER_NONE:
                    print_repeated(" ", col_widths[i]);
            }
        }
        else
        {
            Cell *parent = below_row->cells[i].parent;
            char *str = NULL;
            size_t len = get_line_of_cell(parent, line_indices[i], &str);
            line_indices[i]++;
            print_padded(str,
                len,
                get_total_width(table, col_widths, i, parent->settings.span_x),
                0,
                parent->settings.align);
            i += parent->settings.span_x - 1;
        }
    }
    printf("\n");
}

void print_debug(Table *table)
{
    size_t col_widths[table->num_cols];
    size_t row_heights[table->num_rows];
    get_dimensions(table, col_widths, row_heights);

    printf("Num rows: %zu, Num cols: %zu, Dimensions: ", table->num_rows, table->num_cols);
    for (size_t i = 0; i < table->num_cols; i++) printf("%zu ", col_widths[i]);
    printf("; ");
    for (size_t i = 0; i < table->num_rows; i++) printf("%zu ", row_heights[i]);
    printf("\n");
}

/*
Summary: Prints table to stdout
*/
void print_table(Table *table)
{
    size_t col_widths[table->num_cols];
    size_t row_heights[table->num_rows];
    get_dimensions(table, col_widths, row_heights);

    //print_debug(table);

    size_t line_indices[table->num_cols];
    for (size_t i = 0; i < table->num_cols; i++) line_indices[i] = 0;

    // Print rows
    struct Row *prev_row = NULL;
    struct Row *curr_row = table->first_row;
    size_t row_index = 0;
    while (curr_row != NULL)
    {
        if (curr_row->hline)
        {
            print_complete_line(table,
                prev_row,
                curr_row,
                line_indices,
                col_widths);
        }

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
            for (size_t k = 0; k < table->num_cols; k += curr_row->cells[k].settings.span_x)
            {
                if (table->vlines[k])
                {
                    switch (curr_row->cells[k].settings.border_left)
                    {
                        case BORDER_SINGLE:
                            printf("%s", BORDER_MATRIX_SINGLE[VLINE_INDEX]);
                            break;
                        case BORDER_DOUBLE:
                            printf("%s", BORDER_MATRIX_DOUBLE[VLINE_INDEX]);
                            break;
                        case BORDER_NONE:
                            printf(" ");
                    }
                }

                char *str;
                size_t str_len;
                str = NULL;
                str_len = get_line_of_cell(&curr_row->cells[k], line_indices[k], &str);
                
                print_padded(str,
                    str_len,
                    get_total_width(table, col_widths, k, curr_row->cells[k].settings.span_x),
                    curr_row->cells[k].dot_padding,
                    curr_row->cells[k].settings.align);
                
                line_indices[k]++;
            }

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
    for (size_t i = 0; i < MAX_COLS; i++)
    {
        res->cells[i] = (Cell){
            .is_set = false,
            .x = i,
            .y = y,
            .parent = NULL,
            .text = NULL,
            .settings = get_settings_align(ALIGN_LEFT)
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

void hline_internal(Table *table, BorderStyle style, struct Row *row)
{
    for (size_t i = 0; i < table->num_cols; i++)
    {
        row->cells[i].settings.border_above = style;
    }
    row->hline = style != BORDER_NONE;
}

void vline_internal(Table *table, BorderStyle style, size_t col)
{
    if (col >= MAX_COLS) return;

    struct Row *curr_row = table->first_row;
    while (curr_row != NULL)
    {
        curr_row->cells[col].settings.border_left = style;
        curr_row = curr_row->next_row;
    }
    table->vlines[col] = style != BORDER_NONE;
    if (col >= table->num_cols) table->num_cols = col + 1;
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
        .vlines = { false }
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

/*
Summary: Adds next cell. Buffer is not copied. print_table will access it.
    Ensure that lifetime of buffer outlasts last call of print_table!
*/
void add_cell(Table *table, CellSettings settings, char *text)
{
    add_cell_internal(table, settings, text, false);
}

void add_cell_at(Table *table, size_t x, size_t y, CellSettings settings, char *text)
{
    struct Row *temp_row = table->curr_row;
    size_t temp_col = table->curr_col;
    set_position(table, x, y);
    add_cell_internal(table, settings, text, false);
    table->curr_row = temp_row;
    table->curr_col = temp_col;
}

void add_standard_cell(Table *table, char *text)
{
    add_cell_internal(table, get_standard_settings(), text, false);
}

/*
Summary: Same as add_cell, but frees buffer on reset
*/
void add_managed_cell(Table *table, CellSettings settings, char *text)
{
    add_cell_internal(table, settings, text, true);
}

void add_empty_cell(Table *table)
{
    add_cell_internal(table, get_settings_align(ALIGN_LEFT), NULL, false);
}

void add_standard_cell_fmt(Table *table, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    v_add_cell_fmt(table, get_standard_settings(), fmt, args);
    va_end(args);
}

/*
Summary: Adds next cell and maintains buffer of text.
    Use add_cell to save memory if you maintain a content string yourself.
*/
void add_cell_fmt(Table *table, CellSettings settings, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    v_add_cell_fmt(table, settings, fmt, args);
    va_end(args);
}

void v_add_cell_fmt(Table *table, CellSettings settings, char *fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    char *buffer = malloc(needed * sizeof(char));
    vsnprintf(buffer, needed, fmt, args_copy);
    add_cell_internal(table, settings, buffer, true);
    va_end(args_copy);
}

/*
Summary: Puts contents of memory-contiguous 2D array into table cell by cell.
    Strings are not copied. Ensure that lifetime of array outlasts last call of print_table.
    Position of next insertion is first cell in next row.
*/
void add_from_array(Table *table, size_t width, size_t height, TextAlignment *col_aligns, char **array)
{
    if (table->curr_col + width > MAX_COLS) return;
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            add_cell(table, get_settings_align(col_aligns[j]), *(array + i * width + j));
        }
        next_row(table);
    }
}

/*
Summary: Inserts num hlines at supplied indices (pass them as int!)
*/
void horizontal_line(Table *table, BorderStyle style, size_t num, ...)
{
    va_list args;
    va_start(args, num);
    for (size_t i = 0; i < num; i++)
    {
        size_t index = (size_t)va_arg(args, int);
        struct Row *row = NULL;
        if (index < table->num_rows)
        {
            row = get_row(table, index);
        }
        else
        {
            while (index >= table->num_rows)
            {
                row = append_row(table);
            }
        }
        hline_internal(table, style, row);
    }
    va_end(args);
}

/*
Summary: Inserts num vlines at supplied indices (pass them as int!)
*/
void vertical_line(Table *table, BorderStyle style, size_t num, ...)
{
    va_list args;
    va_start(args, num);
    for (size_t i = 0; i < num; i++)
    {
        size_t col = (size_t)va_arg(args, int);
        if (col < MAX_COLS) vline_internal(table, style, col);
    }
    va_end(args);
}

/*
Summary: Puts enclosing borders around current content of table
*/
void make_boxed(Table *table, BorderStyle style)
{
    horizontal_line(table, style, 2, 0, table->num_rows);
    vertical_line(table, style, 2, 0, table->num_cols);
    struct Row *last_row = get_row(table, table->num_rows - 1);
    last_row->cells[0].settings.border_left = BORDER_NONE;
    last_row->cells[table->num_cols - 1].settings.border_left = BORDER_NONE;
}

void set_alignment_of_col(Table *table, TextAlignment align, size_t col, bool exclude_header)
{
    struct Row *row;
    if (exclude_header)
    {
        if (table->first_row != NULL)
        {
            row = table->first_row->next_row;
        }
        else
        {
            row = NULL;
        }
    }
    else
    {
        row = table->first_row;
    }

    while (row != NULL)
    {
        row->cells[col].settings.align = align;
        row = row->next_row;
    }
}

CellSettings get_standard_settings()
{
    return get_settings_align(ALIGN_LEFT);
}

CellSettings get_settings_align(TextAlignment align)
{
    return get_settings_align_span(align, 1, 1);
}

CellSettings get_settings_align_span(TextAlignment align, size_t span_x, size_t span_y)
{
    return get_settings_align_span_border(align, span_x, span_y, BORDER_NONE, BORDER_NONE);
}

CellSettings get_settings_align_span_border(TextAlignment align, size_t span_x, size_t span_y, BorderStyle border_left, BorderStyle border_above)
{
    return (CellSettings){
        .align = align,
        .span_x = span_x,
        .span_y = span_y,
        .border_left = border_left,
        .border_above = border_above
    };
}

void change_settings(Table *table, CellSettings settings)
{
    table->curr_row->cells[table->curr_col].settings = settings;
}

void change_settings_at(Table *table, size_t x, size_t y, CellSettings settings)
{
    if (x >= table->num_cols) return;
    struct Row *row = get_row(table, y);
    if (row == NULL) return;
    row->cells[x].settings = settings;
}
