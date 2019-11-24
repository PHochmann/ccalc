#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "table.h"
#include "../string_util.h"

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

// Constraint solving for calculating minimum dimension needed for spanning cells

struct Constraint
{
    size_t from_index;
    size_t to_index;
    size_t min;
};

size_t needed_to_satisfy(struct Constraint *constr, size_t *vars)
{
    size_t sum = 0;
    for (size_t i = constr->from_index; i < constr->to_index; i++)
    {
        sum += vars[i];
    }
    return sum < constr->min ? constr->min - sum : 0;
}

// Make sure to zero out result before
void satisfy_constraints(size_t num_constrs, struct Constraint *constrs, size_t *result)
{
    // Current heuristic: Do simple cells before, then split remaining amount evenly

    for (size_t i = 0; i < num_constrs; i++)
    {
        if (constrs[i].to_index - constrs[i].from_index == 1)
        {
            result[constrs[i].from_index] = constrs[i].min;
        }
    }

    for (size_t i = 0; i < num_constrs; i++)
    {
        size_t needed = needed_to_satisfy(&constrs[i], result);
        if (needed > 0)
        {
            size_t length = constrs[i].to_index - constrs[i].from_index;
            size_t adjustment = needed / length;
            for (size_t j = 0; j < length; j++)
            {
                result[constrs[i].from_index + j] += adjustment;
                if (needed % length > j) result[constrs[i].from_index + j]++;
            }
        }
    }
}

// String and printf helper functions

void print_repeated(char *string, size_t times)
{
    for (size_t i = 0; i < times; i++) printf("%s", string);
}

void print_padded_substr(char *string, int length, int total_length, TextAlignment textpos)
{
    if (string == NULL)
    {
        printf("%*s", total_length, "");
        return;
    }

    // Lengths passed to printf are including color codes
    // We need to adjust the total length to include them
    int ansi_length = ansi_strlen(string);
    int adjusted_total_len = total_length + length - ansi_length;

    switch (textpos)
    {
        case ALIGN_LEFT:
        {
            printf("%-*.*s", adjusted_total_len, length, string);
            break;
        }
        case ALIGN_RIGHT:
        {
            printf("%*.*s", adjusted_total_len, length, string);
            break;
        }
        case ALIGN_CENTER:
        {
            int padding = (total_length - ansi_length) / 2;
            printf("%*s%.*s%*s", padding, "", length, string,
                (total_length - ansi_length) % 2 == 0 ? padding : padding + 1, "");
        }
    }
}

size_t get_num_lines(char *string)
{
    size_t res = 1;
    size_t pos = 0;
    while (string[pos] != '\0')
    {
        if (string[pos] == '\n')
        {
            res++;
        }
        pos++;
    }
    return res;
}

// Returns: Length of line (excluding \n or \0)
size_t get_line_of_string(char *string, size_t line_index, char **out_start)
{
    if (string == NULL)
    {
        return 0;
    }

    // Search for start of line
    if (line_index > 0)
    {
        while (*string != '\0')
        {
            string++;
            if (*string == '\n')
            {
                line_index--;
                if (line_index == 0)
                {
                    string++;
                    break;
                }
            }
        }
    }

    // String does not have that much lines
    if (line_index != 0)
    {
        return 0;
    }

    *out_start = string;

    // Count length of line
    size_t count = 0;
    while (string[count] != '\0' && string[count] != '\n')
    {
        count++;
    }
    return count;
}

void add_cell_internal(Table *table, TextAlignment align, size_t span_x, size_t span_y, char *text, bool needs_free)
{
    if (table->curr_col + span_x > MAX_COLS)
    {
        // next_line needed
        return;
    }

    struct Cell *cell = &table->curr_row->cells[table->curr_col];
    *cell = (struct Cell){
        .is_set = true,
        .align = align,
        .span_x = span_x,
        .span_y = span_y,
        .text = text,
        .text_needs_free = needs_free,
        .text_height = get_num_lines(text),
        .text_width = 0,
        .child_type = CTYPE_NONE,
        .parent = NULL
    };

    // Set child cells for span_x
    table->curr_row->is_empty = false;
    for (size_t i = 1; i < span_x; i++)
    {
        if (!table->curr_row->cells[table->curr_col + i].is_set)
        {
            table->curr_row->cells[table->curr_col + i] = (struct Cell){
                .is_set = true,
                .child_type = CTYPE_FROM_LEFT,
                .parent = cell,
                .span_y = span_y
            };
        }
        else
        {
            // Span clash: truncate span_x
            cell->span_x = i;
            break;
        }
    }

    // Insert rows and set child cells for span_y
    struct Row *curr_row = table->curr_row;
    size_t curr_col = table->curr_col;
    for (size_t i = 0; i < span_y - 1; i++)
    {
        next_row(table);
        table->curr_row->is_empty = false;
        if (!table->curr_row->cells[curr_col].is_set)
        {
            table->curr_row->cells[curr_col] = (struct Cell){
                .is_set = true,
                .child_type = CTYPE_FROM_ABOVE,
                .parent = cell,
                .span_x = span_x
            };
        }
        else
        {
            // Span clashes with already set cell, truncate it
            cell->span_y = i + 1;
            break;
        }
    }
    table->curr_row = curr_row;
    table->curr_col = curr_col;

    // Calculate width of text
    for (size_t i = 0; i < cell->text_height; i++)
    {
        char *line;
        get_line_of_string(text, i, &line);
        size_t line_width = ansi_strlen(line);
        if (cell->text_width < line_width) cell->text_width = line_width;
    }

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

void get_dimensions(Table *table, size_t *col_widths, size_t *row_heights)
{
    size_t num_cells_upper = table->num_cols * table->num_rows;
    struct Constraint constrs[num_cells_upper];

    // Satisfy constraints of width
    struct Row *curr_row = table->first_row;
    size_t index = 0;
    while (curr_row != NULL)
    {
        for (size_t i = 0; i < MAX_COLS; i++)
        {
            // Build constraints for set parent cells
            if (curr_row->cells[i].is_set && curr_row->cells[i].child_type == CTYPE_NONE)
            {
                size_t min = curr_row->cells[i].text_width;
                // Constraint can be weakened when vlines are in between
                for (size_t j = i + 1; j < i + curr_row->cells[i].span_x; j++)
                {
                    if (min == 0) break;
                    if (table->vlines[j] != BORDER_NONE) min--;
                }

                constrs[index] = (struct Constraint){
                    .min = min,
                    .from_index = i,
                    .to_index = i + curr_row->cells[i].span_x
                };
                index++;
            }
        }
        curr_row = curr_row->next_row;
    }
    for (size_t i = 0; i < table->num_cols; i++) col_widths[i] = 0;
    satisfy_constraints(index, constrs, col_widths);

    // Satisfy constraints of height
    curr_row = table->first_row;
    index = 0;
    size_t row_index = 0;
    while (curr_row != NULL)
    {
        for (size_t i = 0; i < MAX_COLS; i++)
        {
            if (curr_row->cells[i].is_set && curr_row->cells[i].child_type == CTYPE_NONE)
            {
                size_t min = curr_row->cells[i].text_height;
                struct Row *row_checked_for_hline = curr_row->next_row;
                // Constraint can be weakened when hlines are in between
                for (size_t j = i + 1; j < i + curr_row->cells[i].span_y; j++)
                {
                    if (row_checked_for_hline == NULL || min == 0) break;
                    if (row_checked_for_hline->hline_above != BORDER_NONE) min--;
                    row_checked_for_hline = row_checked_for_hline->next_row;
                }

                constrs[index] = (struct Constraint){
                    .min = min,
                    .from_index = row_index,
                    .to_index = row_index + curr_row->cells[i].span_y
                };
                index++;
            }
        }
        curr_row = curr_row->next_row;
        row_index++;
    }
    for (size_t i = 0; i < table->num_rows; i++) row_heights[i] = 0;
    satisfy_constraints(index, constrs, row_heights);
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
            bool above = above_row != NULL
                && above_row->cells[i].child_type != CTYPE_FROM_LEFT;

            bool below = !below_row->is_empty
                && below_row->cells[i].child_type != CTYPE_FROM_LEFT;

            bool left = i > 0
                && below_row->cells[i - 1].child_type != CTYPE_FROM_ABOVE;

            bool right = i < MAX_COLS - 1
                && below_row->cells[i].child_type != CTYPE_FROM_ABOVE;

            print_border_char(left, right, above, below, below_row->hline_above);
        }

        // Print -- in between intersections (or content when cell has span_y > 1)
        if (below_row->cells[i].child_type != CTYPE_FROM_ABOVE)
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
            char *str = NULL;
            size_t len = get_line_of_cell(&below_row->cells[i], line_indices[i], &str);
            line_indices[i]++;
            print_padded_substr(str,
                len,
                get_total_width(table, col_widths, i, below_row->cells[i].span_x),
                below_row->cells[i].parent->align);
        }
    }

    // Print last vline-hline intersection on right border
    if (table->vlines[table->num_cols] != BORDER_NONE)
    {
        bool above = above_row != NULL;
        bool below = !below_row->is_empty;
        bool left  = below_row->cells[table->num_cols - 1].child_type != CTYPE_FROM_ABOVE;
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

    printf("Dimensions:");
    for (size_t i = 0; i < table->num_cols; i++) printf("%zu ", col_widths[i]);
    printf(", ");
    for (size_t i = 0; i < table->num_rows; i++) printf("%zu ", row_heights[i]);
    printf("\n");
    //return;

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

        for (size_t j = 0; j < table->num_cols; j++)
        {
            if (curr_row->cells[j].child_type != CTYPE_FROM_ABOVE)
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
                
                print_padded_substr(str,
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

struct Row *malloc_row()
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
    while (row->next_row != NULL) row = row->next_row;
    row->next_row = malloc_row();
    table->num_rows++;
    return row->next_row;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ User-functions ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/*
Summary: Next inserted cell will be inserted at first unset column of next row.
*/
void next_row(Table *table)
{
    if (table == NULL) return;

    // Extend linked list if necessary
    if (table->curr_row->next_row == NULL)
    {
        table->curr_row = append_row(table);
        table->curr_col = 0;
    }
    else
    {
        table->curr_row = table->curr_row->next_row;
        size_t i = 0;
        while (i < MAX_COLS && table->curr_row->cells[i].is_set)
        {
            i++;
        }
        table->curr_col = i;
    }
}

/*
Returns: A new table with a single, empty row.
*/
Table get_empty_table()
{
    struct Row *first_row = malloc_row();
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
    Position of next insertion is next cell in last row.
*/
void add_cells_from_array(Table *table, size_t width, size_t height, char **array, TextAlignment *alignments)
{
    if (width > MAX_COLS) return;

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