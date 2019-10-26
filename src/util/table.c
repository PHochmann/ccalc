#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "table.h"

#define MAX_CELL_LINE 100

struct Cell
{
    TextPosition textpos;
    char *text;
};

struct TableState
{
    // Table dimensions
    size_t num_cols;
    size_t num_rows;
    // Marker which cell is inserted next
    size_t x;
    size_t y;
    // Max. ocurring widths and heights in columns and rows
    size_t col_widths[MAX_COLS];
    size_t row_heights[MAX_ROWS];

    size_t num_hlines;
    size_t hlines[MAX_ROWS];
    struct Cell cells[MAX_COLS][MAX_ROWS];
};

struct TableState state;

size_t strlen_ansi(char *string)
{
    size_t res = 0;
    size_t pos = 0;
    while (string[pos] != '\0' && string[pos] != '\n')
    {
        if (string[pos] == '\x1B')
        {
            while (string[pos] != 'm')
            {
                pos++;
            }
        }
        else
        {
            res++;
        }
        pos++;
    }
    return res;
}

void print_repeated(char *string, size_t amount)
{
    for (size_t i = 0; i < amount; i++) printf("%s", string);
}

void print_padded(char *string, size_t total_length, TextPosition textpos)
{
    if (string == NULL)
    {
        print_repeated(" ", total_length);
        return;
    }

    size_t str_length = strlen_ansi(string);

    switch (textpos)
    {
        case TEXTPOS_LEFT:
            printf("%s", string);
            print_repeated(" ", total_length - str_length);
            break;
        case TEXTPOS_RIGHT:
            print_repeated(" ", total_length - str_length);
            printf("%s", string);
            break;
        case TEXTPOS_CENTER:
            print_repeated(" ", (total_length - str_length) / 2);
            printf("%s", string);
            print_repeated(" ", ceil((double)(total_length - str_length) / 2));
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

size_t get_line(char *string, size_t line_index, char *out_buffer)
{
    if (string == NULL)
    {
        *out_buffer = '\0';
        return 0;
    }

    size_t start = 0;
    size_t str_length = strlen(string);

    while (line_index != 0 && start < str_length)
    {
        start++;
        if (string[start] == '\n')
        {
            line_index--;
            if (line_index == 0)
            {
                start++;
                break;
            }
        }
    }

    // String does not have that much lines
    // Return empty string
    if (line_index != 0)
    {
        *out_buffer = '\0';
        return 0;
    }

    size_t count = 0;
    while (string[start + count] != '\0' && string[start + count] != '\n')
    {
        out_buffer[count] = string[start + count];
        count++;
    }
    out_buffer[count] = '\0';
    return count;
}

void get_dimensions(char *string, size_t *out_length, size_t *out_height)
{
    *out_length = 0;
    *out_height = get_num_lines(string);
    for (size_t i = 0; i < *out_height; i++)
    {
        char buffer[strlen(string) + 1];
        get_line(string, i, buffer);
        size_t line_length = strlen_ansi(buffer);
        if (*out_length < line_length) *out_length = line_length;
    }
}

void reset_table()
{
    for (size_t i = 0; i < MAX_COLS; i++)
    {
        state.col_widths[i] = 0;
        for (size_t j = 0; j < MAX_ROWS; j++)
        {
            free(state.cells[i][j].text);
            state.cells[i][j].text = NULL;
        }
    }

    for (size_t j = 0; j < MAX_ROWS; j++)
    {
        state.row_heights[j] = 0;
    }

    state.num_hlines = 0;
    state.x = 0;
    state.y = 0;
    state.num_cols = 0;
    state.num_rows = 0;
}

void add_cell(TextPosition textpos, char *fmt, ...)
{
    if (state.x >= MAX_COLS || state.y >= MAX_ROWS) return;

    struct Cell *cell = &state.cells[state.x][state.y];

    va_list args;
    va_start(args, fmt);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    cell->text = malloc(needed * sizeof(char));
    va_end(args);
    va_start(args, fmt);
    vsnprintf(cell->text, needed, fmt, args);
    va_end(args);

    cell->textpos = textpos;

    // Update book-keeping info
    size_t length = 0;
    size_t height = 0;
    get_dimensions(cell->text, &length, &height);
    if (length > state.col_widths[state.x]) state.col_widths[state.x] = length;
    if (height > state.row_heights[state.y]) state.row_heights[state.y] = height;
    if (state.num_cols <= state.x) state.num_cols = state.x + 1;
    if (state.num_rows <= state.y) state.num_rows = state.y + 1;
    state.x++;
}

void add_cells_from_array(size_t x, size_t y, size_t width, size_t height, char *array[height][width], ...)
{
    va_list alignments;

    for (size_t i = 0; i < height; i++)
    {
        set_position(x, y + i);
        va_start(alignments, array);
        for (size_t j = 0; j < width; j++)
        {
            add_cell(va_arg(alignments, TextPosition), "%s", array[i][j]);
        }
        va_end(alignments);
    }
}

void next_row()
{
    state.y++;
    state.x = 0;
}

// Inserts horizontal line above current row
void hline()
{
    if (state.num_hlines + 1 < MAX_ROWS)
    {
        state.hlines[state.num_hlines++] = state.y;
    }
}

void set_position(size_t col, size_t row)
{
    if (col >= MAX_COLS || row >= MAX_ROWS) return;
    state.x = col;
    state.y = row;
}

void print_table(bool borders)
{
    size_t hlines = 0;

    if (borders)
    {
        printf("┌");
        for (size_t i = 0; i < state.num_cols; i++)
        {
            print_repeated("─", state.col_widths[i]);
            if (i != state.num_cols - 1) printf("┬");
        }
        printf("┐\n");
    }
    // - - -

    // Print cells
    for (size_t i = 0; i < state.num_rows; i++)
    {
        if (borders)
        {
            while (hlines < state.num_hlines && state.hlines[hlines] == i)
            {
                hlines++;
                printf("├");
                for (size_t k = 0; k < state.num_cols; k++)
                {
                    print_repeated("─", state.col_widths[k]);
                    if (k != state.num_cols - 1) printf("┼");
                }
                printf("┤\n");
            }
        }

        for (size_t j = 0; j < state.row_heights[i]; j++)
        {
            if (borders) printf("│");

            for (size_t k = 0; k < state.num_cols; k++)
            {
                char cell_line[MAX_CELL_LINE];
                get_line(state.cells[k][i].text, j, cell_line);
                print_padded(cell_line, state.col_widths[k], state.cells[k][i].textpos);
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
        for (size_t i = 0; i < state.num_cols; i++)
        {
            print_repeated("─", state.col_widths[i]);
            if (i != state.num_cols - 1) printf("┴");
        }
        printf("┘\n");
    }
    // - - -
}
