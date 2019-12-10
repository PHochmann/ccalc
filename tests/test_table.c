#include <stdlib.h>
#include <stdio.h>

#include "test_table.h"
#include "../src/string_util.h"
#include "../src/table/table.h"

#define NUM_CASES 2
#define GREEN     "\x1B[92m"
#define CYAN      "\x1B[1;36m"
#define YELLOW    "\x1B[33;1m"
#define RED       "\x1B[31;1m"
#define COL_RESET "\x1B[0m"

char *arrayA[4][4] = {
    { "alpha", "beta", "gamma", " delta " },
    { "1", YELLOW "-1110.1" COL_RESET, "a.......", " 777" },
    { "2", "10.1", "b", " 222" },
    { "3.......", RED "23.1132310" COL_RESET, "c", " 333" },
};

bool table_test()
{
    // Case 1
    Table t1 = get_empty_table();
    add_from_array(&t1, 4, 4, (TextAlignment[]){ ALIGN_LEFT, ALIGN_NUMBERS, ALIGN_RIGHT, ALIGN_CENTER }, (char**)arrayA);
    change_settings_at(&t1, 1, 0, get_settings_align(ALIGN_CENTER));
    set_position(&t1, 4, 0);
    add_standard_cell(&t1, " test ");
    set_position(&t1, 3, 4);
    add_cell(&t1, "!");
    set_position(&t1, 3, 5);
    add_cell(&t1, "span x\nand y"); 
    set_position(&t1, 0, 4);
    add_cell(&t1, "span x");
    add_cell(&t1, "span y\nspan y\nspan y\nspan y\nspan y");
    next_row(&t1);
    add_cell(&t1, GREEN "span x" COL_RESET);
    next_row(&t1);
    add_cell(&t1, CYAN "span x" COL_RESET);
    make_boxed(&t1, BORDER_SINGLE);
    horizontal_line(&t1, BORDER_DOUBLE, 3, 1, 4, 5);
    horizontal_line(&t1, BORDER_SINGLE, 1, 6);
    vertical_line(&t1, BORDER_DOUBLE, 4, 1, 2, 3, 4);
    vertical_line(&t1, BORDER_DOUBLE, 1, 5);
    print_table(&t1);
    free_table(&t1);

    // Case 2
    Table t2 = get_empty_table();
    add_cell(&t2, " ");
    next_row(&t2);
    add_cell(&t2, " ");
    add_standard_cell_fmt(&t2, "%d", 1);
    add_standard_cell_fmt(&t2, "%d", 2);
    add_standard_cell_fmt(&t2, "%d", 3);
    add_cell(&t2, " ");
    next_row(&t2);
    add_standard_cell_fmt(&t2, "%d", 4);
    add_standard_cell_fmt(&t2, "%d", 5);
    add_standard_cell_fmt(&t2, "%d", 6);
    next_row(&t2);
    add_standard_cell_fmt(&t2, "%d", 7);
    add_standard_cell_fmt(&t2, "%d", 8);
    add_standard_cell_fmt(&t2, "%d", 9);
    next_row(&t2);
    add_cell(&t2, " ");
    vertical_line(&t2, BORDER_SINGLE, 6, 0, 1, 2, 3, 4, 5);
    horizontal_line(&t2, BORDER_SINGLE, 6, 0, 1, 2, 3, 4, 5);
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
