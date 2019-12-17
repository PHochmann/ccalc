#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "test_parser.h"
#include "test_tree_to_string.h"
#include "test_randomized.h"
#include "test_table.h"
#include "../src/commands/core.h"
#include "../src/arithmetics/arith_context.h"
#include "../src/table/table.h"

#if TEST_TABLES
static const size_t NUM_TESTS = 4;
#else
static const size_t NUM_TESTS = 3;
#endif

static Test (*test_getters[])() = {
    get_parser_test,
    get_tree_to_string_test,
    get_randomized_test,
    get_table_test,
};

int main()
{
    arith_init_ctx();

    Table table = get_empty_table();
    set_default_alignments(&table, 4,
        (TextAlignment[]){ ALIGN_RIGHT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_LEFT });
    add_empty_cell(&table);
    set_vline(&table, BORDER_SINGLE);
    override_left_border(&table, BORDER_NONE);
    override_alignment(&table, ALIGN_CENTER);
    add_cell(&table, " Test suite ");
    override_alignment(&table, ALIGN_CENTER);
    add_cell(&table, " #Cases ");
    override_alignment(&table, ALIGN_CENTER);
    add_cell(&table, " Result ");
    next_row(&table);
    set_hline(&table, BORDER_SINGLE);

    bool error = false;
    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        if (i == 0) override_above_border(&table, BORDER_SINGLE);
        add_cell_fmt(&table, " %zu ", i + 1);
        add_cell_fmt(&table, " %s ", test.name);
        add_cell_fmt(&table, " %d ", test.num_cases);
        if (test.suite())
        {
            add_cell(&table, F_GREEN " passed " COL_RESET);
        }
        else
        {
            add_cell(&table, F_RED " failed " COL_RESET);
            error = true;
        }
        next_row(&table);
    }

    set_span(&table, 3, 1);
    override_alignment(&table, ALIGN_CENTER);
    set_hline(&table, BORDER_SINGLE);
    add_cell(&table, "Overall result");
    add_cell(&table, error ? F_RED " failed " COL_RESET : F_GREEN " passed " COL_RESET);
    next_row(&table);
    make_boxed(&table, BORDER_SINGLE);
    print_table(&table);
    free_table(&table);

    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
