#include <stdlib.h>
#include <stdio.h>

#include "printing.h"
#include "table.h"
#include "text.h"
#include "constraint.h"

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

size_t get_total_width(Table *table, size_t *col_widths, size_t col, size_t span_x)
{
    size_t sum = 0;
    for (size_t i = 0; i < span_x; i++)
    {
        if (i < span_x - 1 && table->border_left_counters[col + i + 1] > 0) sum++;
        sum += col_widths[col + i];
    }
    return sum;
}

size_t get_line_of_cell(struct Cell *cell, size_t line_index, char **out_start)
{
    if (cell->parent == NULL) return get_line_of_string(cell->text, line_index, out_start);
    return get_line_of_string(cell->parent->text, line_index, out_start);
}

TextAlignment get_align(TextAlignment default_align, struct Cell *cell)
{
    if (cell->parent != NULL) return get_align(default_align, cell->parent);
    if (cell->override_align)
    {
        return cell->align;
    }
    else
    {
        return default_align;
    }
}

size_t get_span_x(struct Cell *cell)
{
    if (cell->parent != NULL)
    {
        return cell->parent->span_x;
    }
    else
    {
        return cell->span_x;
    }
}

BorderStyle get_border_above(BorderStyle default_style, struct Cell *cell)
{
    if (cell->override_border_above)
    {
        return cell->border_above;
    }
    else
    {
        return default_style;
    }
}

BorderStyle get_border_left(BorderStyle default_style, struct Cell *cell)
{
    if (cell->override_border_left)
    {
        return cell->border_left;
    }
    else
    {
        return default_style;
    }
}

void count_styles(BorderStyle style, size_t *out_num_single, size_t *out_num_double)
{
    if (style == BORDER_SINGLE) (*out_num_single)++;
    if (style == BORDER_DOUBLE) (*out_num_double)++;
}

void print_intersection_char(BorderStyle default_right_border_left,
    BorderStyle default_below_border_above,
    struct Cell *right_above, struct Cell *left_below, struct Cell *right_below)
{
    size_t num_single = 0;
    size_t num_double = 0;

    BorderStyle above = BORDER_NONE;
    if (right_above != NULL)
    {
        above = get_border_left(default_right_border_left, right_above);
        count_styles(above, &num_single, &num_double);
    }

    BorderStyle right = BORDER_NONE;
    if (right_below != NULL)
    {
        right = get_border_above(default_below_border_above, right_below);
        count_styles(right, &num_single, &num_double);
    }

    BorderStyle below = BORDER_NONE;
    if (right_below != NULL)
    {
        below = get_border_left(default_right_border_left, right_below);
        count_styles(below, &num_single, &num_double);
    }

    BorderStyle left  = BORDER_NONE;
    if (left_below != NULL)
    {
        left = get_border_above(default_below_border_above, left_below);
        count_styles(left, &num_single, &num_double);
    }

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

    if (num_double > num_single)
    {
        printf("%s", BORDER_MATRIX_DOUBLE[BORDER_LOOKUP[index]]);
    }
    else
    {
        printf("%s", BORDER_MATRIX_SINGLE[BORDER_LOOKUP[index]]);
    }
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
        if (table->border_left_counters[i] > 0)
        {
            print_intersection_char(table->borders_left[i],
                below_row->border_above,
                above_row != NULL ? &above_row->cells[i] : NULL,
                i > 0 ? &below_row->cells[i - 1] : NULL,
                &below_row->cells[i]);
        }

        // Print hline in between intersections (or content when cell has span_y > 1)
        if (below_row->cells[i].parent == NULL || below_row->cells[i].parent->y == below_row->cells[i].y)
        {
            switch (get_border_above(below_row->border_above, &below_row->cells[i]))
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
            struct Cell *parent = below_row->cells[i].parent;
            char *str = NULL;
            size_t len = get_line_of_cell(parent, line_indices[i], &str);
            line_indices[i]++;
            print_padded(str,
                len,
                get_total_width(table, col_widths, i, parent->span_x),
                0,
                get_align(table->alignments[i], parent));
            i += parent->span_x - 1;
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

void print_table_internal(Table *table)
{
    size_t col_widths[table->num_cols];
    size_t row_heights[table->num_rows];
    get_dimensions(table, col_widths, row_heights);

    //print_debug(table);

    // Special cases: If last row/col is empty, delete all vlines/hlines in it
    if (row_heights[table->num_rows - 1] == 0)
    {
        for (size_t i = 0; i < table->num_cols; i++)
        {
            table->curr_col = i;
            override_left_border(table, BORDER_NONE);
        }
    }
    if (col_widths[table->num_cols - 1] == 0)
    {
        struct Row *row = table->first_row;
        table->curr_col = table->num_cols - 1;
        while (row != NULL)
        {
            table->curr_row = row;
            override_above_border(table, BORDER_NONE);
            row = row->next_row;
        }
    }
    // - -

    size_t line_indices[table->num_cols];
    for (size_t i = 0; i < table->num_cols; i++) line_indices[i] = 0;

    // Print rows
    struct Row *prev_row = NULL;
    struct Row *curr_row = table->first_row;
    size_t row_index = 0;
    while (curr_row != NULL)
    {
        if (curr_row->border_above_counter > 0)
        {
            print_complete_line(table,
                prev_row,
                curr_row,
                line_indices,
                col_widths);
        }

        // Reset line indices for newly beginning cells, don't reset them for cells that are children spanning from above
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
            for (size_t k = 0; k < table->num_cols; k += get_span_x(&curr_row->cells[k]))
            {
                if (table->border_left_counters[k] > 0)
                {
                    switch (get_border_left(table->borders_left[k], &curr_row->cells[k]))
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

                char *str = NULL;
                size_t str_len = get_line_of_cell(&curr_row->cells[k], line_indices[k], &str);
                print_padded(str,
                    str_len,
                    get_total_width(table, col_widths, k, get_span_x(&curr_row->cells[k])),
                    curr_row->cells[k].dot_padding,
                    get_align(table->alignments[k], &curr_row->cells[k]));
                
                line_indices[k]++;
            }

            printf("\n");
        }

        row_index++;
        prev_row = curr_row;
        curr_row = curr_row->next_row;
    }
}
