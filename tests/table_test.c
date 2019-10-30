#include <stdlib.h>
#include <stdio.h>

#include "table_test.h"
#include "../src/util/string_util.h"
#include "../src/util/table.h"

#define NUM_CASES 3

/*
┌───────────────────────────────────────────────┬─────────────────────┬─────────┬────────────────────────────────────────────────┐
│                                          lorem│ipsum                │dolor sit│amet                                            │
│                    consectetur adipisici elit,│                     │         │                                                │
│sed eiusmod tempor incidunt ut labore et dolore│                     │         │                                                │
│                                  magna aliqua.│                     │         │                                                │
│                                             Ut│enim ad minim veniam,│         │quis nostrud exercitation ullamco               │
│                                               │                     │         │laboris nisi ut aliquid ex ea commodi consequat.│
├───────────────────────────────────────────────┼─────────────────────┼─────────┼────────────────────────────────────────────────┤
│                                               │        Quis         │  aute   │                                                │
└───────────────────────────────────────────────┴─────────────────────┴─────────┴────────────────────────────────────────────────┘
┌┬┬┬────┐
├┼┼┼────┤
├┼┼┼────┤
├┼┼┼────┤
││││Test│
└┴┴┴────┘
┌──────────────────┐
│        !         │
│       test       │
│verycolorfulstring│
└──────────────────┘
*/

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
    printf("%zu\n", ansi_strlen(F_RED " error " COL_RESET));
    Table table;

    // Case 1
    add_cells_from_array(&table, 0, 0, 4, 3, arrayA, TEXTPOS_RIGHT, TEXTPOS_LEFT, TEXTPOS_LEFT, TEXTPOS_LEFT);
    add_cells_from_array(&table, 1, 3, 2, 1, arrayB, TEXTPOS_CENTER, TEXTPOS_CENTER);
    hline(&table);
    print_table(&table, true);
    reset_table(&table);

    // Case 2
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

    // Test is not automatic - ask user if tables look right
    printf("\nDoes this look right to you [Y/n]? ");

    char input = getchar();
    if (input == '\n' || input == 'y' || input == 'Y')
    {
        return true;
    }
    else
    {
        return false;
    }
}

Test get_table_test()
{
    return (Test){
        table_test,
        NUM_CASES,
        "Table"
    };
}
