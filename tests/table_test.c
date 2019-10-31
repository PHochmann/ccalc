#include <stdlib.h>
#include <stdio.h>

#include "table_test.h"
#include "../src/util/string_util.h"
#include "../src/util/table.h"

#define NUM_CASES 3

char *arrayA[3][4] = {
    { "lorem", "ipsum", "dolor sit", "amet" },
    { "consectetur adipisici elit,\n"
      "sed eiusmod tempor incidunt ut labore et dolore\n"
      "magna aliqua.", "", "", "" },
    { "Ut" , "enim ad minim veniam,", "", "quis nostrud exercitation ullamco\n"
        "laboris nisi ut aliquid ex ea commodi consequat."}
};

char *arrayB[1][2] = {
    { "Quis", "aute" }
};

bool table_test()
{
    Table table = get_table();

    // Case 1
    add_cells_from_array(&table, 0, 0, 4, 3, arrayA, TEXTPOS_RIGHT, TEXTPOS_LEFT, TEXTPOS_LEFT, TEXTPOS_LEFT);
    add_cells_from_array(&table, 1, 3, 2, 1, arrayB, TEXTPOS_CENTER, TEXTPOS_CENTER);
    hline(&table);
    print_table(&table, true);
    print_table(&table, false);
    reset_table(&table);

    // Case 2
    add_cell(&table, TEXTPOS_CENTER, "Test");
    next_row(&table);
    hline(&table);
    hline(&table);
    hline(&table);
    set_position(&table, 3, 3);
    add_cell(&table, TEXTPOS_CENTER, "Test");
    print_table(&table, true);
    reset_table(&table);

    // Case 3
    add_cell_fmt(&table, TEXTPOS_CENTER, "%s", "!");
    next_row(&table);
    add_cell(&table, TEXTPOS_CENTER, "test");
    next_row(&table);
    add_cell(&table, TEXTPOS_CENTER, OP_COLOR "very" COL_RESET VAR_COLOR "colorful" COL_RESET CONST_COLOR "string" COL_RESET);
    print_table(&table, true);
    reset_table(&table);

    // Case 4
    print_table(&table, true);

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
