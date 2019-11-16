#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "table.h"
#include "../string_util.h"

void print_repeated(char *string, int times)
{
    for (int i = 0; i < times; i++) printf("%s", string);
}

void print_padded_substr(char *string, int length, int total_length, TextPosition textpos)
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
        case TEXTPOS_LEFT:
        {
            printf("%-*.*s", adjusted_total_len, length, string);
            break;
        }
        case TEXTPOS_RIGHT:
        {
            printf("%*.*s", adjusted_total_len, length, string);
            break;
        }
        case TEXTPOS_CENTER:
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
size_t get_line(char *string, size_t line_index, char **out_start)
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

void get_dimensions(char *string, int *out_length, int *out_height)
{
    *out_length = 0;
    *out_height = get_num_lines(string);
    for (int i = 0; i < *out_height; i++)
    {
        char *line;
        get_line(string, i, &line);
        int line_length = ansi_strlen(line);
        if (*out_length < line_length) *out_length = line_length;
    }
}

/*
Returns: A new table with a single, empty row.
*/
Table get_empty_table()
{
    Table res;
    for (size_t i = 0; i < MAX_COLS; i++) res.col_widths[i] = 0;
    res.num_cols = 0;
    res.first_row = calloc(1, sizeof(struct Row));
    res.curr_row = res.first_row;
    return res;
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
            if (curr_row->cells[i].free_on_reset)
            {
                free(curr_row->cells[i].text);
            }
        }

        struct Row *next_row = curr_row->next_row;
        free(curr_row);
        curr_row = next_row;
    }
}

void add_cell_internal(Table *table, TextPosition textpos, char *buffer, bool free_on_reset)
{
    if (table->curr_row->num_cells == MAX_COLS)
    {
        // Already max. amount of cells in row
        return;
    }

    struct Cell *cell = &table->curr_row->cells[table->curr_row->num_cells];
    cell->text = buffer;
    cell->textpos = textpos;
    cell->free_on_reset = free_on_reset;

    // Update book-keeping info
    int length = 0;
    int height = 0;
    get_dimensions(cell->text, &length, &height);

    if (length > table->col_widths[table->curr_row->num_cells])
    {
        table->col_widths[table->curr_row->num_cells] = length;
    }
    if (height > table->curr_row->height)
    {
        table->curr_row->height = height;
    }

    table->curr_row->num_cells++;
    
    if (table->curr_row->num_cells > table->num_cols)
    {
        table->num_cols = table->curr_row->num_cells;
    }
}

/*
Summary: Adds next cell. Buffer is not copied. print_table will access it.
    Ensure that lifetime of buffer outlasts last call of print_table!
*/
void add_cell(Table *table, TextPosition textpos, char *buffer)
{
    add_cell_internal(table, textpos, buffer, false);
}

/*
Summary: Adds next cell and maintains buffer of text.
    Use add_cell to save memory if you maintain a content string yourself.
*/
void add_cell_fmt(Table *table, TextPosition textpos, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    char *buffer = malloc(needed * sizeof(char));
    va_end(args);
    va_start(args, fmt);
    vsnprintf(buffer, needed, fmt, args);
    va_end(args);
    add_cell_internal(table, textpos, buffer, true);
}

/*
Summary: Puts contents of memory-contiguous 2D array into table cell by cell.
    Strings are not copied. Ensure that lifetime of array outlasts last call of print_table.
    Position of next insertion is next cell in last row.
*/
void add_cells_from_array(Table *table, size_t width, size_t height, char **array, TextPosition *alignments)
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
Summary: Next inserted cell will be inserted at first column of next row.
*/
void next_row(Table *table)
{
    struct Row *res = calloc(1, sizeof(struct Row));
    table->curr_row->next_row = res;
    table->curr_row = res;
}

/*
Summary: Inserts horizontal line above current row.
*/
void hline(Table *table)
{
    table->curr_row->hline_above = true;
}

/*
Summary: Prints table to stdout, optionally with border-box around it
*/
void print_table(Table *table, bool print_borders)
{
    if (print_borders)
    {
        printf("┌");
        for (size_t i = 0; i < table->num_cols; i++)
        {
            print_repeated("─", table->col_widths[i]);
            if (i != table->num_cols - 1) printf("┬");
        }
        printf("┐\n");
    }

    struct Row *curr_row = table->first_row;
    while (curr_row != NULL)
    {
        if (curr_row->hline_above)
        {
            if (print_borders) printf("├");
            for (size_t k = 0; k < table->num_cols; k++)
            {
                print_repeated("─", table->col_widths[k]);
                if (print_borders && k != table->num_cols - 1) printf("┼");
            }
            if (print_borders) printf("┤");
            printf("\n");
        }

        for (int j = 0; j < curr_row->height; j++)
        {
            if (print_borders) printf("│");

            for (size_t k = 0; k < table->num_cols; k++)
            {
                char *str = NULL;
                int len = get_line(curr_row->cells[k].text, j, &str);
                print_padded_substr(str, len, table->col_widths[k], curr_row->cells[k].textpos);
                if (print_borders) printf("│");
            }
            printf("\n");
        }

        curr_row = curr_row->next_row;
    }

    if (print_borders)
    {
        printf("└");
        for (size_t i = 0; i < table->num_cols; i++)
        {
            print_repeated("─", table->col_widths[i]);
            if (i != table->num_cols - 1) printf("┴");
        }
        printf("┘\n");
    }
}
