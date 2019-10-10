#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "table.h"

struct TableState
{
    size_t num_cols;
    size_t num_rows;
    size_t curr_col;
    size_t curr_row;
    size_t col_widths[MAX_COLS];
    char *cells[MAX_COLS][MAX_ROWS];
};

struct TableState state;

void print_repeated(char *string, size_t amount)
{
    for (size_t i = 0; i < amount; i++) printf("%s", string);
}

void print_padded(char *string, size_t total_length)
{
    size_t length = 0;
    if (string != NULL)
    {
        printf("%s", string);
        length = strlen(string);
    }
    print_repeated(" ", total_length - length);
}

void print_centered(char *string, size_t total_length)
{
    size_t diff = total_length;
    if (string != NULL) diff -= strlen(string);
    print_repeated(" ", diff / 2);
    if (string != NULL) printf("%s", string);
    print_repeated(" ", ceil((double)diff / 2));
}

void update(size_t length)
{
    if (length > state.col_widths[state.curr_col]) state.col_widths[state.curr_col] = length;
    if (state.num_cols <= state.curr_col) state.num_cols = state.curr_col + 1;
    if (state.num_rows <= state.curr_row) state.num_rows = state.curr_row + 1;
    state.curr_col++;
}

void reset_table()
{
    for (size_t i = 0; i < state.num_cols; i++)
    {
        state.col_widths[i] = 0;
        for (size_t j = 0; j < state.num_rows; j++)
        {
            free(state.cells[i][j]);
            state.cells[i][j] = NULL;
        }
    }

    state.curr_col = 0;
    state.curr_row = 0;
    state.num_cols = 0;
    state.num_rows = 0;
}

void add_cell(char *fmt, ...)
{
    if (state.curr_col >= MAX_COLS || state.curr_row >= MAX_ROWS) return;

    va_list args;
    va_start(args, fmt);
    size_t length = vsnprintf(NULL, 0, fmt, args);
    state.cells[state.curr_col][state.curr_row] = malloc((length + 1) * sizeof(char));
    va_end(args);
    va_start(args, fmt);
    vsnprintf(state.cells[state.curr_col][state.curr_row], length + 1, fmt, args);
    va_end(args);
    update(length);
}

void add_cell_from_buffer(char *buffer)
{
    size_t length = strlen(buffer);
    state.cells[state.curr_col][state.curr_row] = malloc((length + 1) * sizeof(char));
    strcpy(state.cells[state.curr_col][state.curr_row], buffer);
    update(length);
}

void add_cells_from_array(size_t x, size_t y, size_t width, size_t height, char *array[height][width])
{
    for (size_t i = 0; i < height; i++)
    {
        set_position(x, y + i);
        for (size_t j = 0; j < width; j++)
        {
            add_cell("%s", array[i][j]);
        }
    }
}

void next_row()
{
    state.curr_row++;
    state.curr_col = 0;
}

void set_position(size_t col, size_t row)
{
    if (col >= MAX_COLS || row >= MAX_ROWS) return;
    state.curr_col = col;
    state.curr_row = row;
}

void print_table(bool head_border)
{
    printf("┌");
    for (size_t i = 0; i < state.num_cols; i++)
    {
        print_repeated("─", state.col_widths[i] + 2);
        if (i != state.num_cols - 1) printf("┬");
    }
    printf("┐\n");
    // - - -

    // Print cells
    for (size_t i = 0; i < state.num_rows; i++)
    {
        if (i == 1 && head_border) // Print table head border
        {
            printf("├");
            for (size_t j = 0; j < state.num_cols; j++)
            {
                print_repeated("─", state.col_widths[j] + 2);
                if (j != state.num_cols - 1) printf("┼");
            }
            printf("┤\n");
        }

        printf("│");
        for (size_t j = 0; j < state.num_cols; j++)
        {
            printf(" ");
            if (i == 0 && head_border)
            {
                print_centered(state.cells[j][i], state.col_widths[j]);
            }
            else
            {
                print_padded(state.cells[j][i], state.col_widths[j]);
            }
            printf(" │");
        }
        printf("\n");
    }
    // - - -

    // Print bottom border
    printf("└");
    for (size_t i = 0; i < state.num_cols; i++)
    {
        print_repeated("─", state.col_widths[i] + 2);
        if (i != state.num_cols - 1) printf("┴");
    }
    printf("┘\n");
    // - - -
}
