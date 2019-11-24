#include <stdlib.h>
#include <stdio.h>

#include "test_table.h"
#include "../src/string_util.h"
#include "../src/commands/table.h"

#define NUM_CASES 5
#define GREEN     "\x1B[92m"
#define CYAN      "\x1B[1;36m"
#define COL_RESET "\x1B[0m"

char *arrayA[4][3] = {
    { "alpha", "beta", "gamma" },
    { "1", "1110.1", "a......." },
    { "2", "10.1", "b" },
    { "3", "23.1132310", "c" },
};

char *arrayB[1][2] = {
    { "centered", F_GREEN "centered" COL_RESET }
};

bool table_test()
{
    Table table;

    // Case 1
    table = get_empty_table();
    add_cells_from_array(&table, 3, 4, (char**)arrayA, (TextAlignment[]){ ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT });
    hline(&table, BORDER_DOUBLE);
    next_row(&table);
    add_cells_from_array(&table, 2, 1, (char**)arrayB, (TextAlignment[]){ ALIGN_CENTER, ALIGN_CENTER });
    next_row(&table);
    hline(&table, BORDER_SINGLE);
    add_cell(&table, ALIGN_CENTER, "");
    print_table(&table);
    print_table(&table); // Case 2
    free_table(&table);

    // Case 3
    table = get_empty_table();
    add_cell(&table, ALIGN_CENTER, "1st row");
    next_row(&table);
    hline(&table, BORDER_DOUBLE);
    next_row(&table);
    hline(&table, BORDER_DOUBLE);
    add_cell(&table, ALIGN_CENTER, "3rd row");
    print_table(&table);
    free_table(&table);

    // Case 4
    table = get_empty_table();
    add_cell_fmt(&table, ALIGN_CENTER, "%s", "!");
    next_row(&table);
    add_cell(&table, ALIGN_CENTER, "test");
    next_row(&table);
    add_cell(&table, ALIGN_CENTER,
        GREEN "very" COL_RESET CYAN "colorful" COL_RESET GREEN "string" COL_RESET);
    print_table(&table);
    free_table(&table);

    // Case 5
    table = get_empty_table();
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
