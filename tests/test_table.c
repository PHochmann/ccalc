#include <stdlib.h>
#include <stdio.h>

#include "test_table.h"
#include "../src/string_util.h"
#include "../src/table/table.h"

#define NUM_CASES 1

#define GREEN     "\x1B[92m"
#define CYAN      "\x1B[1;36m"
#define YELLOW    "\x1B[33;1m"
#define RED       "\x1B[31;1m"
#define COL_RESET "\x1B[0m"

char *should_be = "┌──────────┬───────────────┬──────────┬───────┬──────┐\n"
                  "│  alpha   │     " YELLOW "beta" COL_RESET "      │  gamma   │ delta │ test │\n"
                  "├══════════┼═══════════════┼══════════┼═══════┼══════┤\n"
                  "│ 1        │" YELLOW " -1110.1000000 " COL_RESET "│ a....... │  777  │      │\n"
                  "│ 2        │    10.0000000 │        b │  222  │      │\n"
                  "│ 3....... │" RED "    23.1132310 " COL_RESET "│        c │  333  │      │\n"
                  "├──────────┴───────────────┼──────────┼───────┼──────┤\n"
                  "│ span x                   │ span y   │   !   │      │\n"
                  "├──────────────────────────┤ span y   │       └──────┤\n"
                  "│" GREEN " span x" COL_RESET "                   │ span y   │  ^ no border │\n"
                  "├══════════════════════════┘ span y   │   and span x │\n"
                  "│" CYAN " span x" COL_RESET "                     < span y │   and also y │\n"
                  "└─────────────────────────────────────┴──────────────┘";

char *arrayA[4][4] = {
    { "alpha", YELLOW "beta" COL_RESET, "gamma", " delta " },
    { " 1 ", YELLOW " -1110.1 " COL_RESET, "a....... ", " 777" },
    { " 2 ", " 10 ", "b ", " 222" },
    { " 3....... ", RED " 23.1132310 " COL_RESET, "c ", " 333" },
};

char *table_test()
{
    printf("Should be:\n%s\nTest case:\n", should_be);
    // Case 1
    Table t1 = get_empty_table();
    set_default_alignments(&t1, 5, (TextAlignment[]){ ALIGN_LEFT, ALIGN_NUMBERS, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_CENTER });
    add_cells_from_array(&t1, 4, 4, (char**)arrayA);
    set_position(&t1, 0, 0);
    override_alignment_of_row(&t1, ALIGN_CENTER);
    set_position(&t1, 4, 0);
    set_vline(&t1, 4, BORDER_SINGLE);
    add_cell(&t1, " test ");
    set_position(&t1, 2, 1);
    set_vline(&t1, 2, BORDER_SINGLE);
    set_hline(&t1, BORDER_DOUBLE);
    set_position(&t1, 3, 4);
    set_vline(&t1, 3, BORDER_SINGLE);
    add_cell(&t1, "!");
    set_position(&t1, 3, 5);
    set_span(&t1, 2, 2);
    override_alignment(&t1, ALIGN_RIGHT);
    override_above_border(&t1, BORDER_NONE);
    add_cell(&t1, " ^ no border \n and span x \n and also y ");
    set_position(&t1, 0, 4);
    set_hline(&t1, BORDER_SINGLE);
    set_span(&t1, 2, 1);
    add_cell(&t1, " span x");
    override_alignment(&t1, ALIGN_LEFT);
    set_vline(&t1, 4, BORDER_SINGLE);
    set_span(&t1, 1, 3);
    add_cell(&t1, " span y \n span y \n span y \n span y \n < span y ");
    next_row(&t1);
    set_hline(&t1, BORDER_SINGLE);
    set_span(&t1, 2, 1);
    add_cell(&t1, GREEN " span x" COL_RESET);
    next_row(&t1);
    set_hline(&t1, BORDER_DOUBLE);
    set_span(&t1, 2, 1);
    add_cell(&t1, CYAN " span x" COL_RESET);
    next_row(&t1);
    set_position(&t1, 1, 6);
    set_vline(&t1, 1, BORDER_SINGLE);
    set_position(&t1, 2, 6);
    override_left_border(&t1, BORDER_NONE);
    make_boxed(&t1, BORDER_SINGLE);
    print_table(&t1);
    free_table(&t1);

    // Test is not automatic - ask user if tables look right
    printf("Do these tables look the same [Y/n]? ");
    char input = getchar();
    printf("\n");

    if (input == '\n' || input == 'y' || input == 'Y')
    {
        return NULL;
    }
    else
    {
        return create_error("Generated table does not look like it should do.\n");
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
