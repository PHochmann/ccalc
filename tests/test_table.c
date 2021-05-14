#include <stdlib.h>
#include <stdio.h>

#include "../src/table/table.h"

#include "test_table.h"

#define NUM_CASES 4

#define GREEN     "\x1B[92m"
#define CYAN      "\x1B[1;36m"
#define YELLOW    "\x1B[33;1m"
#define RED       "\x1B[31;1m"
#define COL_RESET "\x1B[0m"

char *arrayA[4][4] = {
    { "alpha", YELLOW "beta" COL_RESET, "gamma", " delta " },
    { " 1 ", YELLOW " -1110.1 " COL_RESET, "a....... ", " 777" },
    { " 2 ", " 10 ", "b ", " 222" },
    { " 3....... ", RED " 23.1132310 " COL_RESET, "c ", " 333" },
};

bool table_test(__attribute__((unused)) StringBuilder *error_builder)
{
    // Case 1
    Table *t1 = get_empty_table();
    set_default_alignments(t1, 5, (TableHAlign[]){ H_ALIGN_LEFT, H_ALIGN_RIGHT, H_ALIGN_RIGHT, H_ALIGN_CENTER, H_ALIGN_CENTER }, NULL);
    add_cells_from_array(t1, 4, 4, (const char**)arrayA);
    set_position(t1, 0, 0);
    override_horizontal_alignment_of_row(t1, H_ALIGN_CENTER);
    set_position(t1, 4, 0);
    set_vline(t1, 4, BORDER_SINGLE);
    add_cell(t1, " test ");
    set_position(t1, 2, 1);
    set_vline(t1, 2, BORDER_SINGLE);
    set_hline(t1, BORDER_DOUBLE);
    set_position(t1, 3, 4);
    set_vline(t1, 3, BORDER_SINGLE);
    add_cell(t1, "!");
    set_position(t1, 3, 5);
    set_span(t1, 2, 2);
    override_horizontal_alignment(t1, H_ALIGN_RIGHT);
    override_vertical_alignment(t1, V_ALIGN_CENTER);
    override_above_border(t1, BORDER_NONE);
    add_cell(t1, " ^ no border, right aligned \n and span x \n and also y \n and also vertically centered ");
    set_position(t1, 0, 4);
    set_hline(t1, BORDER_SINGLE);
    set_span(t1, 2, 1);
    add_cell(t1, " span x");
    override_horizontal_alignment(t1, H_ALIGN_LEFT);
    set_vline(t1, 4, BORDER_SINGLE);
    set_span(t1, 1, 3);
    override_vertical_alignment(t1, V_ALIGN_BOTTOM);
    add_cell(t1, " bottom aligned \n span y \n < no border ");
    next_row(t1);
    set_hline(t1, BORDER_SINGLE);
    set_span(t1, 2, 1);
    override_horizontal_alignment(t1, H_ALIGN_CENTER);
    add_cell(t1, GREEN " span x" COL_RESET "\n\n");
    next_row(t1);
    set_hline(t1, BORDER_DOUBLE);
    set_span(t1, 2, 1);
    override_horizontal_alignment(t1, H_ALIGN_RIGHT);
    add_cell(t1, CYAN " span x" COL_RESET "\n\n");
    next_row(t1);
    set_position(t1, 1, 6);
    set_vline(t1, 1, BORDER_SINGLE);
    set_position(t1, 2, 6);
    override_left_border(t1, BORDER_NONE);
    make_boxed(t1, BORDER_SINGLE);
    print_table(t1);
    free_table(t1);

    // Case 2
    Table *t2 = get_empty_table();
    for (size_t i = 0; i < TABLE_MAX_COLS - 1; i++)
    {
        for (size_t j = 0; j < TABLE_MAX_COLS - 1; j++)
        {
            override_horizontal_alignment(t2, H_ALIGN_RIGHT);
            add_cell_fmt(t2, " %d ", i * (TABLE_MAX_COLS - 1) + j + 1);
        }
        next_row(t2);
    }
    make_boxed(t2, BORDER_DOUBLE);
    print_table(t2);
    free_table(t2);

    // Case 3
    Table *t3 = get_empty_table();
    next_row(t3);
    print_table(t3);
    free_table(t3);

    // Case 4
    Table *t4 = get_empty_table();
    for (size_t i = 1; i < TABLE_MAX_COLS - 1; i++)
    {
        set_span(t4, i, 1);
        add_cell_fmt(t4, " x ");
        set_span(t4, TABLE_MAX_COLS - i - 1, 1);
        add_cell_fmt(t4, " x ");
        next_row(t4);
        set_hline(t4, BORDER_SINGLE);
    }

    for (size_t i = 0; i < TABLE_MAX_COLS / 2; i++)
    {
        set_span(t4, 2, 1);
        add_cell_fmt(t4, " x ");
    }

    next_row(t4);
    set_all_vlines(t4, BORDER_SINGLE);
    make_boxed(t4, BORDER_SINGLE);
    print_table(t4);
    free_table(t4);

    // Case 5
    Table *t5 = get_empty_table();
    set_default_alignments(t5, 3, NULL, (const TableVAlign[]){ V_ALIGN_CENTER, V_ALIGN_TOP, V_ALIGN_TOP });
    add_cells(t5, 3, " henlo ", " smol ", " beans ");
    next_row(t5);
    set_hline(t5, BORDER_DOUBLE);
    set_span(t5, 2, 2);
    override_horizontal_alignment(t5, H_ALIGN_CENTER);
    override_vertical_alignment(t5, V_ALIGN_CENTER);
    add_cell_fmt(t5, " %d ", 666);
    set_span(t5, 2, 1);
    add_cell(t5, " UwU\n OwO ");
    next_row(t5);
    override_left_border(t5, BORDER_NONE);
    add_cell(t5, " >.> ");
    override_above_border(t5, BORDER_NONE);
    add_cell(t5, " <.< ");
    set_hline(t5, BORDER_SINGLE);
    set_all_vlines(t5, BORDER_SINGLE);
    next_row(t5);
    make_boxed(t5, BORDER_SINGLE);
    print_table(t5);
    free_table(t5);
    
    return true;
}

Test get_table_test()
{
    return (Test){
        table_test,
        "Table"
    };
}
