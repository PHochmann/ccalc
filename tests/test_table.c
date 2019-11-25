#include <stdlib.h>
#include <stdio.h>

#include "test_table.h"
#include "../src/string_util.h"
#include "../src/table/table.h"

#define NUM_CASES 1
#define GREEN     "\x1B[92m"
#define CYAN      "\x1B[1;36m"
#define COL_RESET "\x1B[0m"

char *arrayA[4][4] = {
    { "alpha", "beta", "gamma", "delta" },
    { "1", "1110.1", "a.......", "777" },
    { "2", "10.1", "b", "222" },
    { "3.......", "23.1132310", "c", "333" },
};

bool table_test()
{
    Table table;

    // Case 1
    table = get_empty_table();
    add_cells_from_array(&table, 4, 4, (char**)arrayA,
        (TextAlignment[]){ ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT, ALIGN_LEFT });
    set_position(&table, 1, 1);
    vline(&table, BORDER_SINGLE);
    hline(&table, BORDER_SINGLE);
    set_position(&table, 2, 4);
    vline(&table, BORDER_SINGLE);
    hline(&table, BORDER_SINGLE);
    set_position(&table, 3, 0);
    vline(&table, BORDER_SINGLE);
    set_position(&table, 4, 0);
    vline(&table, BORDER_DOUBLE);
    add_cell(&table, ALIGN_LEFT, "test");
    set_position(&table, 3, 4);
    add_cell_span(&table, ALIGN_RIGHT, 2, 3, " span x \nand y ");
    set_position(&table, 0, 4);
    add_cell_span(&table, ALIGN_CENTER, 2, 1, "span x");
    add_cell_span(&table, ALIGN_CENTER, 1, 3, "span y\nspan y\nspan y\nspan y\nspan y");
    next_row(&table);
    hline(&table, BORDER_SINGLE);
    add_cell_span(&table, ALIGN_CENTER, 2, 1, "span x");
    next_row(&table);
    hline(&table, BORDER_SINGLE);
    add_cell_span(&table, ALIGN_CENTER, 2, 1, "span x");
    next_row(&table);
    make_boxed(&table, BORDER_SINGLE);
    print_table(&table);
    free_table(&table);

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
