#include <stdlib.h>
#include <stdio.h>

#include "test_table.h"
#include "../src/string_util.h"
#include "../src/table/table.h"

#define NUM_CASES 2
#define GREEN     "\x1B[92m"
#define CYAN      "\x1B[1;36m"
#define COL_RESET "\x1B[0m"

char *arrayA[4][4] = {
    { "alpha", "beta", "gamma", " delta " },
    { "1", "1110.1", "a.......", "777" },
    { "2", "10.1", "b", "222" },
    { "3.......", "23.1132310", "c", "333" },
};

bool table_test()
{
    // Case 1
    Table t1 = get_empty_table();
    add_cells_from_array(&t1, 4, 4, (char**)arrayA,
        (TextAlignment[]){ ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT, ALIGN_LEFT });
    set_position(&t1, 1, 1);
    vline(&t1, BORDER_SINGLE);
    hline(&t1, BORDER_SINGLE);
    set_position(&t1, 2, 4);
    vline(&t1, BORDER_SINGLE);
    hline(&t1, BORDER_SINGLE);
    set_position(&t1, 3, 0);
    vline(&t1, BORDER_SINGLE);
    set_position(&t1, 4, 0);
    vline(&t1, BORDER_DOUBLE);
    add_cell(&t1, ALIGN_LEFT, " test ");
    set_position(&t1, 3, 4);
    add_cell_span(&t1, ALIGN_CENTER, 2, 3, "~ ~ span x ~ ~\nand y ");
    set_position(&t1, 0, 4);
    add_cell_span(&t1, ALIGN_CENTER, 2, 1, "span x");
    add_cell_span(&t1, ALIGN_CENTER, 1, 3, "span y\nspan y\nspan y\nspan y\nspan y");
    next_row(&t1);
    hline(&t1, BORDER_SINGLE);
    add_cell_span(&t1, ALIGN_CENTER, 2, 1, "span x");
    next_row(&t1);
    hline(&t1, BORDER_SINGLE);
    add_cell_span(&t1, ALIGN_CENTER, 2, 1, "span x");
    next_row(&t1);
    make_boxed(&t1, BORDER_SINGLE);
    print_table(&t1);
    free_table(&t1);

    // Case 2
    Table t2 = get_empty_table();
    add_cell_span(&t2, ALIGN_LEFT, 5, 1, " ");
    next_row(&t2);
    add_cell_span(&t2, ALIGN_LEFT, 1, 3, " ");
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 1);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 2);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 3);
    add_cell_span(&t2, ALIGN_LEFT, 1, 3, " ");
    next_row(&t2);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 4);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 5);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 6);
    next_row(&t2);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 7);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 8);
    add_cell_fmt(&t2, ALIGN_LEFT, "%d", 9);
    next_row(&t2);
    add_cell_span(&t2, ALIGN_LEFT, 5, 1, " ");
    vline_at(&t2, BORDER_DOUBLE, 6, 0, 1, 2, 3, 4, 5);
    hline_at(&t2, BORDER_DOUBLE, 6, 0, 1, 2, 3, 4, 5);
    print_table(&t2);
    free_table(&t2);

    // Test is not automatic - ask user if tables look right
    printf("Does this look right to you [Y/n]? ");
    char input = getchar();
    printf("\n");
    return input == '\n' || input == 'y' || input == 'Y';
}

Test get_table_test()
{
    return (Test){
        table_test,
        NUM_CASES,
        "Table"
    };
}
