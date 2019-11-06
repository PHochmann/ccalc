#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "table.h"
#include "string_util.h"

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
Returns: A new table
*/
Table get_empty_table()
{
    Table res;

    for (size_t i = 0; i < MAX_COLS; i++)
    {
        for (size_t j = 0; j < MAX_ROWS; j++)
        {
            res.cells[i][j].free_on_reset = false;
            res.cells[i][j].text = NULL;
        }
    }

    for (size_t i = 0; i < MAX_COLS; i++) res.col_widths[i] = 0;
    for (size_t i = 0; i < MAX_ROWS; i++) res.row_heights[i] = 0;

    res.num_cols = 0;
    res.num_rows = 0;
    res.x = 0;
    res.y = 0;
    res.num_hlines = 0;

    return res;
}

/*
Summary: Resets all internal values, frees all content strings created by add_cell_fmt
*/
void reset_table(Table *table)
{
    for (size_t i = 0; i < table->num_cols; i++)
    {
        table->col_widths[i] = 0;
        for (size_t j = 0; j < table->num_rows; j++)
        {
            if (table->cells[i][j].free_on_reset)
            {
                free(table->cells[i][j].text);
            }
            table->cells[i][j].text = NULL;
        }
    }

    for (size_t i = 0; i < MAX_ROWS; i++) table->row_heights[i] = 0;
    
    table->num_cols = 0;
    table->num_rows = 0;
    table->x = 0;
    table->y = 0;
    table->num_hlines = 0;
}

void add_cell_internal(Table *table, TextPosition textpos, char *buffer, bool free_on_reset)
{
    if (table->x >= MAX_COLS || table->y >= MAX_ROWS) return;

    struct Cell *cell = &table->cells[table->x][table->y];
    cell->text = buffer;
    cell->textpos = textpos;

    // Update book-keeping info
    int length = 0;
    int height = 0;
    get_dimensions(cell->text, &length, &height);
    if (length > table->col_widths[table->x]) table->col_widths[table->x] = length;
    if (height > table->row_heights[table->y]) table->row_heights[table->y] = height;
    if (table->num_cols <= table->x) table->num_cols = table->x + 1;
    if (table->num_rows <= table->y) table->num_rows = table->y + 1;
    table->cells[table->x][table->y].free_on_reset = free_on_reset;
    table->x++;
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
Summary: Puts contents of array into table cell by cell. Strings are not copied!
    Ensure that lifetime of array outlasts last call of print_table!
    Position of next insertion is next cell in last row
*/
void add_cells_from_array(Table *table, size_t x, size_t y, size_t width, size_t height, char *array[height][width], ...)
{
    va_list alignments;

    for (size_t i = 0; i < height; i++)
    {
        set_position(table, x, y + i);
        va_start(alignments, array);
        for (size_t j = 0; j < width; j++)
        {
            add_cell(table, va_arg(alignments, TextPosition), array[i][j]);
        }
        va_end(alignments);
    }
}

void next_row(Table *table)
{
    table->y++;
    table->x = 0;
}

/*
Summary: Inserts horizontal line above current row
    Must be set in a sorted order, i.e. don't use it after using set_pos to go to a previous row.
*/
void hline(Table *table)
{
    if (table->num_hlines + 1 < MAX_ROWS)
    {
        table->hlines[table->num_hlines++] = table->y;
    }
}

/*
Summary: Changes position of next cell to be added
*/
void set_position(Table *table, size_t col, size_t row)
{
    if (col >= MAX_COLS || row >= MAX_ROWS) return;
    table->x = col;
    table->y = row;
}

/*
Summary: Prints table to stdout
*/
void print_table(Table *table, bool borders)
{
    size_t hlines = 0;

    if (borders)
    {
        printf("┌");
        for (size_t i = 0; i < table->num_cols; i++)
        {
            print_repeated("─", table->col_widths[i]);
            if (i != table->num_cols - 1) printf("┬");
        }
        printf("┐\n");
    }
    // - - -

    // Print cells
    for (size_t i = 0; i < table->num_rows; i++)
    {
        if (borders)
        {
            while (hlines < table->num_hlines && table->hlines[hlines] == i)
            {
                hlines++;
                printf("├");
                for (size_t k = 0; k < table->num_cols; k++)
                {
                    print_repeated("─", table->col_widths[k]);
                    if (k != table->num_cols - 1) printf("┼");
                }
                printf("┤\n");
            }
        }

        for (int j = 0; j < table->row_heights[i]; j++)
        {
            if (borders) printf("│");

            for (size_t k = 0; k < table->num_cols; k++)
            {
                char *str = NULL;
                int len = get_line(table->cells[k][i].text, j, &str);
                print_padded_substr(str, len, table->col_widths[k], table->cells[k][i].textpos);
                if (borders) printf("│");
            }
            printf("\n");
        }
    }
    // - - -

    // Print bottom border
    if (borders)
    {
        printf("└");
        for (size_t i = 0; i < table->num_cols; i++)
        {
            print_repeated("─", table->col_widths[i]);
            if (i != table->num_cols - 1) printf("┴");
        }
        printf("┘\n");
    }
    // - - -
}
