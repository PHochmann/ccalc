#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../src/commands/core.h"
#include "../src/arithmetics/arith_context.h"
#include "../src/util/table.h"

#include "test.h"
#include "parser_test.h"
#include "tree_to_string_test.h"
#include "randomized_test.h"
#include "table_test.h"

#if TEST_TABLES
static const size_t NUM_TESTS = 4;
#else
static const size_t NUM_TESTS = 4;
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
    Table table = get_table();
    add_cell(&table, TEXTPOS_LEFT, "");
    add_cell(&table, TEXTPOS_CENTER, " Test suite ");
    add_cell(&table, TEXTPOS_CENTER, " #Cases ");
    add_cell(&table, TEXTPOS_CENTER, " Result ");
    next_row(&table);
    hline(&table);

    bool error = false;
    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        add_cell_fmt(&table, TEXTPOS_LEFT, " %zu ", i);
        add_cell_fmt(&table, TEXTPOS_LEFT, " %s ", test.name);
        add_cell_fmt(&table, TEXTPOS_RIGHT, " %d ", test.num_cases);
        if (test.suite())
        {
            add_cell(&table, TEXTPOS_LEFT, F_GREEN " passed " COL_RESET);
        }
        else
        {
            add_cell(&table, TEXTPOS_LEFT, F_RED " failed " COL_RESET);
            error = true;
        }
        next_row(&table);
    }

    print_table(&table, true);
    reset_table(&table);

    if (!error)
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}
