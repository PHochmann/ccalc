#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "test_tree_util.h"
#include "test_parser.h"
#include "test_tree_to_string.h"
#include "test_randomized.h"
#include "test_table.h"
#include "test_transformation.h"
#include "../src/commands/commands.h"
#include "../src/table/table.h"

static const size_t NUM_TESTS = 6;
static Test (*test_getters[])() = {
    get_tree_util_test,
    get_parser_test,
    get_tree_to_string_test,
    get_randomized_test,
    get_table_test,
    get_transformation_test,
};

int main()
{
    Table table = get_empty_table();
    set_default_alignments(&table, 4,
        (TextAlignment[]){ ALIGN_RIGHT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_LEFT });
    add_empty_cell(&table);
    set_vline(&table, 0, BORDER_SINGLE);
    override_left_border(&table, BORDER_NONE);
    add_cell(&table, " Test suite ");
    add_cell(&table, " #Cases ");
    add_cell(&table, " Result ");
    override_alignment_of_row(&table, ALIGN_LEFT);
    next_row(&table);
    set_hline(&table, BORDER_SINGLE);

    bool error = false;
    StringBuilder error_builder = get_stringbuilder(100);
    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        if (i == 0) override_above_border(&table, BORDER_SINGLE);
        add_cell_fmt(&table, " %zu ", i + 1);
        add_cell_fmt(&table, " %s ", test.name);
        add_cell_fmt(&table, " %d ", test.num_cases);

        if (test.suite(&error_builder))
        {
            add_cell(&table, F_GREEN " passed " COL_RESET);
        }
        else
        {
            printf("[" F_RED "%s" COL_RESET "] %s", test_getters[i]().name, error_builder.buffer);
            add_cell(&table, F_RED " failed " COL_RESET);
            error = true;
        }
        next_row(&table);


    }
    free_stringbuilder(&error_builder);

    set_span(&table, 3, 1);
    override_alignment(&table, ALIGN_CENTER);
    set_hline(&table, BORDER_DOUBLE);
    add_cell(&table, " End result ");
    add_cell(&table, error ? F_RED " failed " COL_RESET : F_GREEN " passed " COL_RESET);
    next_row(&table);
    make_boxed(&table, BORDER_SINGLE);
    print_table(&table);
    free_table(&table);

    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
